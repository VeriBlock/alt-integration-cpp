from ...framework.test_framework import PopIntegrationTestFramework


class PopParamsTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        pop_params = self.nodes[0].getpopparams()
        bootstrap_block = pop_params.bootstrapBlock
        bootstrap_hash = bootstrap_block.hash

        try:
            self.nodes[0].getblock(bootstrap_hash)
            self.log.info("Test passed: bootstrap block={} exists!".format(bootstrap_hash))
            return
        except:
            pass

        try:
            reversed_bootstrap_hash = bytes.fromhex(bootstrap_hash)[::-1].hex()
            self.nodes[0].getblock(reversed_bootstrap_hash)
            self.log.info("Test passed: bootstrap block={} exists (direction B)!".format(reversed_bootstrap_hash))
        except:
            raise Exception('''
            It appears that bootstrap block hash can not be found in local blockchain. 
            Make sure you specified EXISTING Bootstrap block.
            ''')

        # raise Exception('''
        # It appears that bootstrapBlock hash is reversed in getpopparams rpc call.
        # Please, try setting second parameter ("reversedAltHashes") to true/false in ToJSON call
        # for AltChainParams inside getpopparams rpc call.
        # ''')
