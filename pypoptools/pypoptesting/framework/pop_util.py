import time
from typing import List

from .node import Node
from .sync_util import wait_for_block_height

def mine_alt_block(node: Node, nblocks: int):
    for i in range(nblocks):
        node.generate(1)
        tip = node.getbestblock()
        tip_time = tip.time
        current_time = int(time.time())
        if current_time < tip_time:
            time.sleep(tip_time - current_time)

# size = size of chain to be created
def create_endorsed_chain(node: Node, apm, size: int, addr: str = None) -> None:
    block = node.getbestblock()
    initial_height = block.height

    for height in range(initial_height, initial_height + size):
        atv_id = endorse_block(node, apm, height, addr)
        mine_alt_block(node, nblocks=1)
        # endorsing prev tip
        wait_for_block_height(node, height + 1)
        containing_block = node.getbestblock()
        assert atv_id in containing_block.containingATVs, \
            "containing block at height {} does not contain pop tx {}".format(containing_block.height, atv_id)

    wait_for_block_height(node, initial_height + size)


def endorse_block(node: Node, apm, height: int, address: str = None) -> str:
    from pypoptools.pypopminer import PublicationData

    pop_data = node.getpopdatabyheight(height)
    address = address or node.getnewaddress()

    pub = PublicationData()
    pub.header = str(pop_data.header)
    pub.payoutInfo = str(node.getpayoutinfo(address))
    pub.identifier = node.getpopparams().networkId
    pub.contextInfo = str(pop_data.authenticated_context)

    payloads = apm.endorseAltBlock(pub, str(pop_data.last_known_vbk_block))

    for vbk_block in payloads.context:
        node.submitpopvbk(vbk_block.toVbkEncodingHex())
    for vtb in payloads.vtbs:
        node.submitpopvtb(vtb.toVbkEncodingHex())
    for atv in payloads.atvs:
        node.submitpopatv(atv.toVbkEncodingHex())

    return payloads.atvs[0].getId()


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
        mine_alt_block(node, nblocks=(activate - existing))
        wait_for_block_height(node, activate)
