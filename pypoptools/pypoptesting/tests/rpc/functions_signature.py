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

        assert type(functions.get_popdata_by_height) is str and functions.get_popdata_by_height != ""
        assert type(functions.get_popdata_by_hash) is str and functions.get_popdata_by_hash != ""
        assert type(functions.submit_atv) is str and functions.submit_atv != ""
        assert type(functions.submit_vtb) is str and functions.submit_vtb != ""
        assert type(functions.submit_vbk) is str and functions.submit_vbk != ""
        assert type(functions.get_missing_btc_block_hashes) is str and functions.get_missing_btc_block_hashes != ""
        assert type(functions.extract_block_info) is str and functions.extract_block_info != ""
        assert type(functions.get_vbk_block) is str and functions.get_vbk_block != ""
        assert type(functions.get_btc_block) is str and functions.get_btc_block != ""
        assert type(functions.get_vbk_best_block_hash) is str and functions.get_vbk_best_block_hash != ""
        assert type(functions.get_btc_best_block_hash) is str and functions.get_btc_best_block_hash != ""
        assert type(functions.get_raw_atv) is str and functions.get_raw_atv != ""
        assert type(functions.get_raw_vtb) is str and functions.get_raw_vtb != ""

        self.log.info("done _get_rpc_functions()")

    def _get_popdata_by_height(self):
        self.log.info("starting _get_popdata_by_height()")

        func = self.nodes[0].getrpcfunctions().get_popdata_by_height
        res = self.nodes[0].rpc.__getattr__(name=func)(0)

        assert res["block_header"] != None
        assert res["authenticated_context"] != None
        # JSON represantation of the altintegration::AuthenticatedContextInfoContainer, altintegration::ToJSON<AuthenticatedContextInfoContainer>() 
        assert res["authenticated_context"]["serialized"] != None
        assert res["authenticated_context"]["stateRoot"] != None
        assert res["authenticated_context"]["context"] != None
        # JSON represantation of the altintegration::ContextInfoContainer, altintegration::ToJSON<ContextInfoContainer>() 
        assert res["authenticated_context"]["context"]["height"] != None
        assert res["authenticated_context"]["context"]["firstPreviousKeystone"] != None
        assert res["authenticated_context"]["context"]["secondPreviousKeystone"] != None

        self.log.info("done _get_popdata_by_height()")

    def _get_popdata_by_hash(self):
        self.log.info("starting _get_popdata_by_hash()")

        block = self.nodes[0].getbestblock()

        func = self.nodes[0].getrpcfunctions().get_popdata_by_hash
        res = self.nodes[0].rpc.__getattr__(name=func)(block.hash)

        assert res["block_header"] != None
        assert res["authenticated_context"] != None
        # JSON represantation of the altintegration::AuthenticatedContextInfoContainer, altintegration::ToJSON<AuthenticatedContextInfoContainer>() 
        assert res["authenticated_context"]["serialized"] != None
        assert res["authenticated_context"]["stateRoot"] != None
        assert res["authenticated_context"]["context"] != None
        # JSON represantation of the altintegration::ContextInfoContainer, altintegration::ToJSON<ContextInfoContainer>() 
        assert res["authenticated_context"]["context"]["height"] != None
        assert res["authenticated_context"]["context"]["firstPreviousKeystone"] != None
        assert res["authenticated_context"]["context"]["secondPreviousKeystone"] != None

        self.log.info("done _get_popdata_by_hash()")

    def _submit_atv(self):
        self.log.info("starting _submit_atv()")

        self.log.info("done _submit_atv()")

    def _submit_vtb(self):
        self.log.info("starting _submit_vtb()")

        self.log.info("done _submit_vtb()")

    def _submit_vbk(self):
        self.log.info("starting _submit_vbk()")

        self.log.info("done _submit_vbk()")

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