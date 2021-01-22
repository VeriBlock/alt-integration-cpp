import inspect
import time
from typing import List

from pypoptesting.framework.node import Node


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
    """Sets up an RPC connection to the vbitcoind process. Returns False if unable to connect."""
    # Poll at a rate of four times per second
    poll_per_s = 4
    for _ in range(poll_per_s * timeout):
        if node.is_rpc_available():
            return

        time.sleep(1.0 / poll_per_s)
    raise AssertionError("Unable to connect to vbitcoind")
