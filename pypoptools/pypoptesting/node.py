from abc import ABC, abstractmethod

from pypoptesting.entities import *


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
    def getpopdatabyhash(self) -> GetpopdataResponse:
        pass

    @abstractmethod
    def getbtcbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def getvbkbestblockhash(self) -> Hexstr:
        pass

    @abstractmethod
    def getrawatvverbose(self, atvid: Hexstr) -> AtvResponse:
        pass

    @abstractmethod
    def getrawvtbverbose(self, vtbid: Hexstr) -> VtbResponse:
        pass

    @abstractmethod
    def getrawvbkblockverbose(self, vbkblockid: Hexstr) -> VbkBlockResponse:
        pass

    @abstractmethod
    def getrawpopmempool(self) -> RawPopMempoolResponse:
        pass

    @abstractmethod
    def generate(self, nblocks: int):
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
