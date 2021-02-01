from dataclasses import dataclass


@dataclass
class Hexstr:
    value: str

    def __str__(self) -> str:
        return self.value

    def __repr__(self) -> str:
        return self.value


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
    confirmations: int
    endorsedBy: list[Hexstr]
    blockOfProofEndorsements: list[Hexstr]
    containingATVs: list[Hexstr]
    containingVTBs: list[Hexstr]
    containingVBKs: list[Hexstr]


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
    blockOfProofContext: list[BtcBlock]


@dataclass
class VTB:
    id: Hexstr
    tx: VbkPopTx
    containingBlock: GenericBlock


@dataclass
class ATV:
    id: Hexstr
    tx: VbkTx
    blockOfProof: GenericBlock


@dataclass(init=False)
class AtvResponse:
    in_active_chain: bool
    confirmations: int
    blockhash: Hexstr
    blockheight: int
    atv: ATV
    containingBlocks: list[Hexstr]


@dataclass(init=False)
class VtbResponse:
    in_active_chain: bool
    confirmations: int
    blockhash: Hexstr
    blockheight: int
    vtb: VTB
    containingBlocks: list[Hexstr]


@dataclass(init=False)
class VbkBlockResponse:
    in_active_chain: bool
    confirmations: int
    blockhash: Hexstr
    blockheight: int
    vbkblock: VbkBlock
    containingBlocks: list[Hexstr]


@dataclass
class RawPopMempoolResponse:
    vbkblocks: list[Hexstr]
    vtbs: list[Hexstr]
    atvs: list[Hexstr]


@dataclass
class PeerInfo:
    id: int
    banscore: int
