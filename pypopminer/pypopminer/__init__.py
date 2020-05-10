from ._pypopminer import *

def makePublicationData(identifier: int, header: str, payoutInfo: str):
    p = PublicationData()
    p.header = ByteVector(header)
    p.identifier = identifier
    p.payoutInfo = ByteVector(payoutInfo)
    return p
