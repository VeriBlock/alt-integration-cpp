import inspect
import time
from typing import List

from .entities import BlockWithPopData
from .node import Node

NETWORK_ID = 0x3ae6ca
KEYSTONE_INTERVAL = 5


def sync_blocks(nodes: List[Node], *, wait=1, timeout=60):
    """
    Wait until everybody has the same tip.

    sync_blocks needs to be called with an rpc_connections set that has least
    one node already synced to the latest, stable tip, otherwise there's a
    chance it might return before all nodes are stably synced.
    """
    stop_time = time.time() + timeout
    best_hash = None
    while time.time() <= stop_time:
        best_hash = [x.getbestblockhash() for x in nodes]
        if best_hash.count(best_hash[0]) == len(nodes):
            return
        time.sleep(wait)
    raise AssertionError("Block sync timed out:{}".format("".join("\n  {!r}".format(b) for b in best_hash)))


def sync_pop_mempools(nodes: List[Node], *, wait=1, timeout=60):
    """
    Wait until everybody has the same POP data in their POP mempools
    """

    def test(s):
        return s.count(s[0]) == len(nodes)

    stop_time = time.time() + timeout
    atvs = None
    vtbs = None
    vbkblocks = None
    while time.time() <= stop_time:
        mpooldata = [r.getrawpopmempool() for r in nodes]
        atvs = [set(data.atvs) for data in mpooldata]
        vtbs = [set(data.vtbs) for data in mpooldata]
        vbkblocks = [set(data.vbkblocks) for data in mpooldata]

        if test(atvs) and test(vtbs) and test(vbkblocks):
            return
        time.sleep(wait)
    raise AssertionError("POP mempool sync timed out: \natvs: {}\nvtbs: {}\nvbkblocks:{}".format(
        "".join("\n  {!r}".format(m) for m in atvs),
        "".join("\n  {!r}".format(m) for m in vtbs),
        "".join("\n  {!r}".format(m) for m in vbkblocks)
    ))


def sync_pop_tips(nodes: List[Node], *, wait=1, timeout=10):
    """
    Wait until everybody has the same POP TIPS (BTC tip and VBK tip)
    """

    def test(s):
        return s.count(s[0]) == len(nodes)

    btc = None
    vbk = None
    stop_time = time.time() + timeout
    while time.time() <= stop_time:
        btc = [r.getbtcbestblockhash() for r in nodes]
        vbk = [r.getvbkbestblockhash() for r in nodes]

        if test(btc) and test(vbk):
            return
        time.sleep(wait)
    raise AssertionError("POP data sync timed out: \nbtc: {}\nvbk: {}\n".format(
        "".join("\n  {!r}".format(m) for m in btc),
        "".join("\n  {!r}".format(m) for m in vbk),
    ))


def sync_all(nodes: List[Node], **kwargs):
    sync_blocks(nodes, **kwargs)
    sync_pop_mempools(nodes, **kwargs)
    sync_pop_tips(nodes, **kwargs)


def wait_until(predicate, *, attempts=float('inf'), timeout=float('inf'), lock=None):
    if attempts == float('inf') and timeout == float('inf'):
        timeout = 60
    attempt = 0
    time_end = time.time() + timeout

    while attempt < attempts and time.time() < time_end:
        if lock:
            with lock:
                if predicate():
                    return
        else:
            if predicate():
                return
        attempt += 1
        time.sleep(0.05)

    # Print the cause of the timeout
    predicate_source = "''''\n" + inspect.getsource(predicate) + "'''"
    print("wait_until() failed. Predicate: {}".format(predicate_source))
    if attempt >= attempts:
        raise AssertionError("Predicate {} not true after {} attempts".format(predicate_source, attempts))
    elif time.time() >= time_end:
        raise AssertionError("Predicate {} not true after {} seconds".format(predicate_source, timeout))
    raise RuntimeError('Unreachable')


def wait_for_rpc_availability(node: Node, timeout=60) -> None:
    wait_until(lambda: node.is_rpc_available(), timeout=timeout)


# size = size of chain to be created
def create_endorsed_chain(node: Node, apm, size: int, addr: str) -> None:
    block = get_best_block(node)
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


def endorse_block(node: Node, apm, height: int, addr: str) -> str:
    from pypopminer import PublicationData

    pop_data = node.getpopdatabyheight(height)

    pub = PublicationData()
    pub.header = pop_data.header
    pub.payoutInfo = node.getpayoutinfo(addr)
    pub.identifier = NETWORK_ID
    pub.contextInfo = pop_data.authenticated_context

    payloads = apm.endorseAltBlock(pub, pop_data.last_known_vbk_block)

    for vbk_block in payloads.context:
        node.submitpopvbk(vbk_block.toVbkEncodingHex())
    for vtb in payloads.vtbs:
        node.submitpopvtb(vtb.toVbkEncodingHex())
    node.submitpopatv(payloads.atv.toVbkEncodingHex())

    return payloads.atv.getId()


def mine_vbk_blocks(node: Node, apm, amount: int) -> list[str]:
    vbk_blocks = []
    for i in range(amount):
        vbk_block = apm.mineVbkBlocks(1)
        node.submitpopvbk(vbk_block.toVbkEncodingHex())
        vbk_blocks.append(vbk_block.getHash().toHex())
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


def get_best_block(node: Node) -> BlockWithPopData:
    block_hash = node.getbestblockhash()
    return node.getblock(block_hash)


def wait_for_block_height(node: Node, height: int, timeout=60):
    wait_until(lambda: node.getblockcount() >= height, timeout=timeout)
