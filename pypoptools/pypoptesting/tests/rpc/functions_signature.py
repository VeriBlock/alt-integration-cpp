from ...framework.test_framework import PopIntegrationTestFramework


class RpcFunctionsSignatureTest(PopIntegrationTestFramework):
    def set_test_params(self):
        self.num_nodes = 1

    def run_test(self):
        self._get_rpc_functions()

        self._get_popdata_by_height()
        self._get_popdata_by_hash()
        self._submit_atv()
        self._submit_vtb()
        self._submit_vbk()
        self._get_missing_btc_blockhashes()
        self._extract_block_info()
        self._get_popdata_byheight()
        self._get_vbk_block()
        self._get_btc_block()
        self._get_vbk_best_block_hash()
        self._get_btc_best_block_hash()
        self._get_raw_atv()
        self._get_raw_vtb()

    def _get_rpc_functions(self):
        self.log.info("starting _get_rpc_functions()")

        functions = self.nodes[0].getrpcfunctions()
        self.log.info("Validating that getrpcfunctions() returns the full list of needed rpc functions")

        assert functions.get("get_popdata_by_height") != None
        assert functions.get("get_popdata_by_hash") != None
        assert functions.get("submit_atv") != None
        assert functions.get("submit_vtb") != None
        assert functions.get("submit_vbk") != None
        assert functions.get("get_missing_btc_blockhashes") != None
        assert functions.get("extract_block_info") != None
        assert functions.get("get_popdata_byheight") != None
        assert functions.get("get_vbk_block") != None
        assert functions.get("get_btc_block") != None
        assert functions.get("get_vbk_best_block_hash") != None
        assert functions.get("get_btc_best_block_hash") != None
        assert functions.get("get_raw_atv") != None
        assert functions.get("get_raw_vtb") != None



    def _get_popdata_by_height(self):
        self.log.info("starting _get_popdata_by_height()")

    def _get_popdata_by_hash(self):
        self.log.info("starting _get_popdata_by_hash()")

    def _submit_atv(self):
        self.log.info("starting _submit_atv()")

    def _submit_vtb(self):
        self.log.info("starting _submit_vtb()")

    def _submit_vbk(self):
        self.log.info("starting _submit_vbk()")

    def _get_missing_btc_blockhashes(self):
        self.log.info("starting _get_missing_btc_blockhashes()")

    def _extract_block_info(self):
        self.log.info("starting _extract_block_info()")

    def _get_popdata_byheight(self):
        self.log.info("starting _get_popdata_byheight()")

    def _get_vbk_block(self):
        self.log.info("starting _get_vbk_block()")

    def _get_btc_block(self):
        self.log.info("starting _get_btc_block()")

    def _get_vbk_best_block_hash(self):
        self.log.info("starting _get_vbk_best_block_hash()")

    def _get_btc_best_block_hash(self):
        self.log.info("starting _get_btc_best_block_hash()")

    def _get_raw_atv(self):
        self.log.info("starting _get_raw_atv()")

    def _get_raw_vtb(self):
        self.log.info("starting _get_raw_vtb()")