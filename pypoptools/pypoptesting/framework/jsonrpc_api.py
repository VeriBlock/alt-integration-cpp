from requests import post
from requests.auth import HTTPBasicAuth


class JSONRPCException(Exception):
    def __init__(self, rpc_error, http_status=None):
        try:
            errmsg = '%(message)s (%(code)i)' % rpc_error
        except (KeyError, TypeError):
            errmsg = ''
        super().__init__(errmsg)
        self.error = rpc_error
        self.http_status = http_status


class JsonRpcApi:
    def __init__(self, url, user, password):
        self.url = url
        self.auth = HTTPBasicAuth(user, password)
        self.nonce = 0

    def __getattr__(self, name):
        def method(*args, **kwargs):
            payload = {
                "id": self.nonce,
                "method": name,
                "params": list(args) + list(kwargs.values())
            }
            response = post(
                url=self.url,
                auth=self.auth,
                json=payload,
                headers={
                    "Content-type": "application/json"
                }
            )
            self.nonce += 1
            if response.status_code == 200:
                return response.json()['result']
            else:
                raise JSONRPCException(response)

        return method
