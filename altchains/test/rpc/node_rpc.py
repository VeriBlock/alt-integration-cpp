from requests import post
from requests.auth import HTTPBasicAuth
from random import randrange


class NodeRpc:
    def __init__(self, url, user, password):
        self.url = url
        self.user = user
        self.password = password

    def call(self, method, params):
        payload = {
            "jsonrpc": "2.0",
            "id": randrange(20000),
            "method": method,
            "params": params
        }

        response = post(
            url=self.url,
            auth=HTTPBasicAuth(self.user, self.password),
            json=payload
        )

        if response.status_code == 200:
            return response.json()['result']
        else:
            raise Exception(response)

    def getblockchaininfo(self):
        return self.call('getblockchaininfo', [])

    def getblockcount(self):
        return self.call('getblockcount', [])

    def getbestblockhash(self):
        return self.call('getbestblockhash', [])

    def getblockheader(self, block_hash):
        return self.call('getblockheader', [block_hash])

    def getnewaddress(self):
        return self.call('getnewaddress', [])

    def generatetoaddress(self, nblocks, address):
        return self.call('generatetoaddress', [nblocks, address])

    def getbtcbestblockhash(self):
        return self.call('getbtcbestblockhash', [])

    def getvbkbestblockhash(self):
        return self.call('getvbkbestblockhash', [])

    def getpopdata(self, height):
        return self.call('getpopdata', [height])

    def submitpop(self, vbk_blocks, vtbs, atvs):
        return self.call('submitpop', [vbk_blocks, vtbs, atvs])

    def submitpopATV(self, atv):
        return self.call('submitpopATV', [atv])

    def submitpopVBK(self, vbk_block):
        return self.call('submitpopVBK', [vbk_block])

    def submitpopVTB(self, vtb):
        return self.call('submitpopVTB', [vtb])
