from typing import List

from .node import Node
from .sync_util import wait_for_block_height


# size = size of chain to be created
def create_endorsed_chain(node: Node, apm, size: int, addr: str = None) -> None:
    block = node.getbestblock()
    initial_height = block.height

    for height in range(initial_height, initial_height + size):
        atv_id = endorse_block(node, apm, height, addr)
        containing_hash = node.generate(nblocks=1)[0]
        # endorsing prev tip
        wait_for_block_height(node, height + 1)
        containing_block = node.getblock(containing_hash)
        assert atv_id in containing_block.containingATVs, \
            "containing block at height {} does not contain pop tx {}".format(containing_block.height, atv_id)

    wait_for_block_height(node, initial_height + size)


def endorse_block(node: Node, apm, height: int, address: str = None) -> str:
    from pypoptools.pypopminer import PublicationData

    pop_data = node.getpopdatabyheight(height)
    address = address or node.getnewaddress()

    pub = PublicationData()
    pub.header = pop_data.header
    pub.payoutInfo = node.getpayoutinfo(address)
    pub.identifier = node.getpopparams().networkId
    pub.contextInfo = pop_data.authenticated_context

    payloads = apm.endorseAltBlock(pub, pop_data.last_known_vbk_block)

    for vbk_block in payloads.context:
        node.submitpopvbk(vbk_block.toVbkEncodingHex())
    for vtb in payloads.vtbs:
        node.submitpopvtb(vtb.toVbkEncodingHex())
    node.submitpopatv(payloads.atv.toVbkEncodingHex())

    return payloads.atv.getId()


def mine_vbk_blocks(node: Node, apm, amount: int) -> List[str]:
    vbk_blocks = []
    for i in range(amount):
        vbk_block = apm.mineVbkBlocks(1)
        node.submitpopvbk(vbk_block.toVbkEncodingHex())
        vbk_blocks.append(vbk_block.getHash())
    return vbk_blocks


def mine_until_pop_enabled(node: Node):
    existing = node.getblockcount()
    activate = node.getpopparams().popActivationHeight
    assert activate >= 0, "POP security should be able to activate"
    if existing < activate:
        assert activate - existing < 1000, \
            "POP security activates on height {}. Will take too long to activate".format(activate)
        node.generate(nblocks=(activate - existing))
        wait_for_block_height(node, activate)
