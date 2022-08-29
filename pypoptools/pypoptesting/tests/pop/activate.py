"""
Start 2 nodes.
Mine 100 blocks on node0.
Disconnect node[1].
node[0] endorses block 100 (fork A tip).
node[0] mines pop tx in block 101 (fork A tip)

Pop is disabled before block 200 therefore can't handle Pop data
"""
from ...framework.test_framework import PopIntegrationTestFramework
from ...framework.json_rpc import JsonRpcException
from ...framework.pop_util import endorse_block


class PopActivateTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        from pypoptools.pypopminer import MockMiner
        apm = MockMiner()

        assert self.nodes[0].getblockcount() == 0

        activation_height = self.nodes[0].getpopparams().popActivationHeight
        before_activation_height = activation_height - 50
        almost_activation_height = activation_height - 1
        after_activation_height = activation_height + 50

        # mine before activation height
        self.nodes[0].generate(nblocks=before_activation_height)
        assert self.nodes[0].getblockcount() == before_activation_height
        self.log.info("node mined {} blocks".format(before_activation_height))

        # endorse block before activation height
        try:
            endorse_block(self.nodes[0], apm, before_activation_height)
            assert False
        except Exception as e:
            assert isinstance(e, JsonRpcException)
            assert e.error['message'].startswith('POP protocol is not active')
            self.log.info("node failed endorse block {} as expected".format(before_activation_height))

        # mine after activation height
        self.nodes[0].generate(nblocks=after_activation_height - before_activation_height)
        assert self.nodes[0].getblockcount() == after_activation_height
        self.log.info("node mined {} blocks".format(after_activation_height))

        # endorse block before activation height
        endorse_block(self.nodes[0], apm, before_activation_height)
        self.nodes[0].generate(nblocks=1)
        self.log.info("node endorsed block {}".format(before_activation_height))

        # endorse block almost activation height
        endorse_block(self.nodes[0], apm, almost_activation_height)
        self.nodes[0].generate(nblocks=1)
        self.log.info("node endorsed block {}".format(almost_activation_height))

        # endorse block after activation height
        endorse_block(self.nodes[0], apm, after_activation_height)
        self.nodes[0].generate(nblocks=1)
        self.log.info("node endorsed block {}".format(after_activation_height))
