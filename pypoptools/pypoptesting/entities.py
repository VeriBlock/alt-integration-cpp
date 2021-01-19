from dataclasses import dataclass
from typing import List, Optional


@dataclass
class Hexstr:
    value: str


@dataclass
class SubmitPopResponse:
    accepted: bool
    code: str = ""
    message: str = ""


@dataclass
class GetpopdataResponse:
    header: Hexstr
    authenticated_context: Hexstr
    last_known_vbk_block: Hexstr
    last_known_btc_block: Hexstr


@dataclass
class GenericBlock:
    hash: Hexstr
    prevhash: Hexstr
    height: int


@dataclass
class VbkBlock:
    id: Hexstr
    hash: Hexstr
    height: int
    version: int
    previousBlock: Hexstr
    previousKeystone: Hexstr
    secondPreviousKeystone: Hexstr
    merkleRoot: Hexstr
    timestamp: int
    difficulty: int
    nonce: int


@dataclass
class BtcBlock:
    hash: Hexstr


@dataclass
class BlockWithPopData(GenericBlock):
    endorsedBy: List[Hexstr]
    blockOfProofEndorsements: List[Hexstr]
    containingATVs: List[Hexstr]
    containingVTBs: List[Hexstr]
    containingVBKs: List[Hexstr]


@dataclass
class BlockAndNetwork:
    block: GenericBlock
    network: str


@dataclass
class PopParamsResponse:
    popActivationHeight: int
    popRewardPercentage: int
    popRewardCoefficient: int
    popPayoutDelay: int
    bootstrapBlock: GenericBlock
    vbkBootstrap: BlockAndNetwork
    btcBootstrap: BlockAndNetwork
    networkId: int
    maxVbkBlocksInAltBlock: int
    maxVTBsInAltBlock: int
    maxATVsInAltBlock: int
    endorsementSettlementInterval: int
    finalityDelay: int
    keystoneInterval: int
    maxAltchainFutureBlockTime: int


@dataclass
class PublicationData:
    identifier: int
    header: Hexstr
    payoutInfo: Hexstr
    contextInfo: Hexstr


@dataclass
class VbkTx:
    hash: Hexstr
    publicationData: PublicationData


@dataclass
class VbkPopTx:
    hash: Hexstr
    publishedBlock: VbkBlock
    blockOfProof: BtcBlock
    blockOfProofContext: List[BtcBlock]


@dataclass
class VTB:
    id: Hexstr
    tx: VbkPopTx
    containingBlock: GenericBlock


@dataclass
class ATV:
    id: Hexstr
    version: int
    tx: VbkTx
    blockOfProof: GenericBlock


@dataclass
class AtvResponse:
    in_active_chain: bool
    confirmations: int
    block: Optional[GenericBlock]
    atv: ATV
    containingBlocks: List[Hexstr]


@dataclass
class VtbResponse:
    in_active_chain: bool
    confirmations: int
    block: Optional[GenericBlock]
    vtb: VTB
    containingBlocks: List[Hexstr]


@dataclass
class VbkBlockResponse:
    in_active_chain: bool
    confirmations: int
    block: Optional[GenericBlock]
    vbkblock: VbkBlock
    containingBlocks: List[Hexstr]


@dataclass
class RawPopMempoolResponse:
    vbkblocks: List[Hexstr]
    vtbs: List[Hexstr]
    atvs: List[Hexstr]
