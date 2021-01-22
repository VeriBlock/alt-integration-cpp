from pypoptesting.framework.jsonrpc_api import JsonRpcApi

url = "http://127.0.0.1:18332/"
api = JsonRpcApi(url, "vbtc", "vbtc1")
print(api.getblockchaininfo())