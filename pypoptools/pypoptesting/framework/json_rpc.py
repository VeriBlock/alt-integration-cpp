from requests import post
from requests.auth import HTTPBasicAuth


class JsonRpcException(Exception):
    def __init__(self, error, http_status):
        super().__init__('{} (code: {})'.format(error['message'], error['code']))
        self.error = error
        self.http_status = http_status


class JsonRpcApi:
    def __init__(self, url, user=None, password=None):
        self.url = url
        self.nonce = 0
        if user is not None and password is not None:
            self.auth = HTTPBasicAuth(user, password)
        else:
            self.auth = None

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

            response_body = response.json()
            http_status = response.status_code

            if response_body.get('error', None) is not None:
                raise JsonRpcException(response_body['error'], http_status)
            elif 'result' not in response_body:
                raise JsonRpcException({'code': -343, 'message': 'missing JSON-RPC result'}, http_status)
            elif http_status != 200:
                raise JsonRpcException({'code': -342, 'message': 'non-200 HTTP status code'}, http_status)
            else:
                return response_body['result']

        return method
