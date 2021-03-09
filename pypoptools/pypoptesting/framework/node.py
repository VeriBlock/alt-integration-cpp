from abc import ABC, abstractmethod

from .entities import *


class Node(ABC):
    # Control

    @abstractmethod
    def start(self) -> None:
        pass

    @abstractmethod
    def stop(self) -> None:
        pass

    def restart(self) -> None:
        self.stop()
        self.start()

    # Network

    @abstractmethod
    def connect(self, node):
        pass

    @abstractmethod
    def disconnect(self, node):
        pass

    @abstractmethod
    def getpeers(self) -> List[Peer]:
        pass

    # Wallet

    @abstractmethod
    def getbalance(self) -> int:
        pass

    @abstractmethod
    def getnewaddress(self) -> str:
        pass

    @abstractmethod
    def getpayoutinfo(self, address: str = None) -> Hexstr:
        pass

    # Blockchain

    @abstractmethod
    def generate(self, nblocks: int, address: str = None) -> None:
        pass

    def getbestblock(self) -> BlockWithPopData:
        return self.getblock(self.getbestblockhash())

    @abstractmethod
    def getbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def getblock(self, hash: Hexstr) -> BlockWithPopData:
        pass

    @abstractmethod
    def getblockcount(self) -> int:
        pass

    @abstractmethod
    def getblockhash(self, height: int) -> Hexstr:
        pass

    # PoP Mining

    @abstractmethod
    def getbtcbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def getpopdatabyhash(self, hash: Hexstr) -> GetpopdataResponse:
        pass

    @abstractmethod
    def getpopdatabyheight(self, height: int) -> GetpopdataResponse:
        pass

    @abstractmethod
    def getpopparams(self) -> PopParamsResponse:
        pass

    @abstractmethod
    def getrawatv(self, atvid: Hexstr) -> AtvResponse:
        pass

    @abstractmethod
    def getrawpopmempool(self) -> RawPopMempoolResponse:
        pass

    @abstractmethod
    def getrawvbkblock(self, vbkblockid: Hexstr) -> VbkBlockResponse:
        pass

    @abstractmethod
    def getrawvtb(self, vtbid: Hexstr) -> VtbResponse:
        pass

    @abstractmethod
    def getvbkbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def submitpopatv(self, atv: Hexstr) -> SubmitPopResponse:
        pass

    @abstractmethod
    def submitpopvbk(self, vbk: Hexstr) -> SubmitPopResponse:
        pass

    @abstractmethod
    def submitpopvtb(self, vtb: Hexstr) -> SubmitPopResponse:
        pass
