from requests import post
from requests.auth import HTTPBasicAuth


class NodeRpc:
    def __init__(self, url, user, password):
        self.url = url
        self.auth = HTTPBasicAuth(user, password)
        self.nonce = 0

    def generate(self, *args, **kwargs):
        address = self.getnewaddress()
        return self.generatetoaddress(*args, **kwargs, address=address)

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
                json=payload
            )
            self.nonce += 1
            if response.status_code == 200:
                return response.json()['result']
            else:
                raise Exception(response)
        return method
