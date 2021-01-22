from requests import post
from requests.auth import HTTPBasicAuth


class JSONRPCException(Exception):
    def __init__(self, response):
        self.response = response


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
