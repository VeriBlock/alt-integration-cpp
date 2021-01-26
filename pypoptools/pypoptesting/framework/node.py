from abc import ABC, abstractmethod

from .entities import *


class Node(ABC):
    @abstractmethod
    def getpopparams(self) -> PopParamsResponse:
        pass

    @abstractmethod
    def submitpopatv(self, atv: Hexstr) -> SubmitPopResponse:
        pass

    @abstractmethod
    def submitpopvtb(self, vtb: Hexstr) -> SubmitPopResponse:
        pass

    @abstractmethod
    def submitpopvbk(self, vbk: Hexstr) -> SubmitPopResponse:
        pass

    @abstractmethod
    def getpopdatabyheight(self, height: int) -> GetpopdataResponse:
        pass

    @abstractmethod
    def getpopdatabyhash(self, hash: Hexstr) -> GetpopdataResponse:
        pass

    @abstractmethod
    def getbtcbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def getvbkbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def getbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def getrawatv(self, atvid: Hexstr) -> AtvResponse:
        pass

    @abstractmethod
    def getrawvtb(self, vtbid: Hexstr) -> VtbResponse:
        pass

    @abstractmethod
    def getrawvbkblock(self, vbkblockid: Hexstr) -> VbkBlockResponse:
        pass

    @abstractmethod
    def getrawpopmempool(self) -> RawPopMempoolResponse:
        pass

    @abstractmethod
    def generate(self, nblocks: int, address: str) -> None:
        pass

    @abstractmethod
    def getnewaddress(self) -> str:
        pass

    @abstractmethod
    def getpayoutinfo(self, address: Optional[str]) -> Hexstr:
        pass

    @abstractmethod
    def getbalance(self, address: str) -> float:
        pass

    @abstractmethod
    def getblockhash(self, height: int) -> Hexstr:
        pass

    @abstractmethod
    def getblock(self, hash: Hexstr) -> BlockWithPopData:
        pass

    @abstractmethod
    def start(self) -> None:
        pass

    @abstractmethod
    def stop(self) -> None:
        pass

    @abstractmethod
    def restart(self) -> None:
        pass

    @abstractmethod
    def is_started(self) -> bool:
        pass

    @abstractmethod
    def is_rpc_available(self) -> bool:
        pass

    @abstractmethod
    def connect(self, node):
        """
        Should block until this node is connected to other
        :param node: other Node
        """
        pass

    @abstractmethod
    def disconnect(self, node):
        """
        Should block until this node is disconnected from other
        :param node: other node
        """
