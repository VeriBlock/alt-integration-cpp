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

    def getnewaddress(self):
        return self.call('getnewaddress', [])

    def generatetoaddress(self, nblocks, address):
        return self.call('generatetoaddress', [nblocks, address])

    def getpopdata(self, height):
        return self.call('getpopdata', [height])

    def submitpop(self, vbkBlocks, vtbs, atvs):
        return self.call('submitpop', [vbkBlocks, vtbs, atvs])
