"""
Generic Node library.

This module does stuff.
"""


class NodeRPCs:
    @classmethod
    def get_pop_params(self):
        print('getpopparams')
        pass

    @classmethod
    def submit_pop_atv(self):
        print('submitpopatv')
        pass

    @classmethod
    def submit_pop_vtb(self):
        print('submitpopvtb')
        pass

    @classmethod
    def submit_pop_vbk(self):
        print('submitpopvbk')
        pass

    @classmethod
    def get_popdata_by_height(self):
        print('getpopdatabyheight')
        pass

    @classmethod
    def get_popdata_by_hash(self):
        print('getpopdatabyhash')
        pass

    @classmethod
    def get_btc_best_block_hash(self):
        print('getbtcbestblockhash')
        pass

    @classmethod
    def get_vbk_best_block_hash(self):
        print('getvbkbestblockhash')
        pass

    @classmethod
    def get_raw_atv(self):
        print('getrawatv')
        pass

    @classmethod
    def get_raw_vtb(self):
        print('getrawvtb')
        pass

    @classmethod
    def get_raw_vbkblock(self):
        print('getrawvbkblock')
        pass

    @classmethod
    def get_raw_pop_mempool(self):
        print('getrawpopmempool')
        pass

    @classmethod
    def generate_new_blocks(self):
        print('generate_new_blocks')
        pass

    @classmethod
    def get_block_hash_by_height(self):
        print('get_block_hash_by_height')
        pass

    @classmethod
    def get_block_by_hash(self):
        print('get_block_by_hash')
        pass

    @classmethod
    def start(self):
        print('start')
        pass

    @classmethod
    def stop(self):
        print('stop')
        pass

    @classmethod
    def restart(self):
        print('restart')
        pass

    @classmethod
    def is_started(self):
        print('is_started')
        pass

    @classmethod
    def is_rpc_available(self):
        print('is_rpc_available')
        pass


class Node(NodeRPCs):
    @classmethod
    def launch(self):
        print('Start node')
        self.startNode()


def main():
    print('Main')
    Node.launch()


if __name__ == '__main__':
    main()

