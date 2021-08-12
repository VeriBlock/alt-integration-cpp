import inspect
import time
from typing import List

from .node import Node


def start_all(nodes: List[Node]):
    for node in nodes:
        node.start()


def connect_all(nodes: List[Node]):
    # connect each node with each others 
    for i in range(len(nodes)):
        for j in range(i, len(nodes)):
            if i == j:
                continue
            nodes[i].connect(nodes[j])

    def check_nodes_have_peers():
        p = [x.getpeers() for x in nodes]
        return all([len(x) > 0 for x in p])

    # verify that all peers are have at least one peer
    wait_until(check_nodes_have_peers, timeout=60)


def disconnect_all(nodes: List[Node]):
    for i in range(len(nodes)):
        for j in range(len(nodes)):
            if i == j:
                continue
            nodes[i].disconnect(nodes[j])

    def check_nodes_disconnected():
        p = [x.getpeers() for x in nodes]
        return all([len(x) == 0 for x in p])

    # verify that all peers are disconnected
    wait_until(check_nodes_disconnected, timeout=60)


def sync_all(nodes: List[Node], **kwargs):
    sync_blocks(nodes, **kwargs)
    sync_pop_mempools(nodes, **kwargs)
    sync_pop_tips(nodes, **kwargs)


def sync_blocks(nodes: List[Node], *, wait=1, timeout=60, height=None):
    """
    Wait until everybody has the same tip.

    sync_blocks needs to be called with an rpc_connections set that has least
    one node already synced to the latest, stable tip, otherwise there's a
    chance it might return before all nodes are stably synced.
    """
    stop_time = time.time() + timeout
    best_hash = None
    heights = None
    while time.time() <= stop_time:
        heights = [x.getblockcount() for x in nodes]
        best_hash = [x.getbestblockhash() for x in nodes]

        h = set(heights)
        b = set(best_hash)
        # if all best blocks have same hash
        # and heights of best blocks are same
        if len(h) == 1 and len(b) == 1:
            # we do not expect specific height, so return early
            if height is None:
                return

            assert height is not None
            # if height is equal to one that is expected
            if heights.pop() == height:
                return
        time.sleep(wait)

    raise AssertionError(
        "Block sync timed out:{}".format("".join("\n  {}:{!r}".format(h, b) for h, b in zip(heights, best_hash))))


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


def wait_for_block_height(node: Node, height: int, timeout=60):
    wait_until(lambda: node.getblockcount() >= height, timeout=timeout)


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
