package veriblock

// TxType - Veriblock transaction type.
type TxType uint8

// Veriblock transaction type.
const (
	TxTypeVbkTx TxType = iota + 1
	TxTypeVbkPopTx
)

// All constants in alt-cpp.
const (
	Sha256HashSize                      = 32
	BtcTxMaxRawSize                     = 4 * 1000 * 1000
	BtcHeaderSize                       = 80
	HistoryForTimestampAverage          = 20
	VbkMinimumTimestampOnsetBlockHeight = 110000
	MaxHeaderSizePublicationData        = 1024
	MaxPayoutSizePublicationData        = 10000
	MaxContextSizePublicationData       = 10000
	MaxContextCount                     = 15000
	MaxContextCountAltPublication       = 15000
	MaxContextCountVbkPublication       = 15000
	VblakeBlockHashSize                 = 24
	VbkMerkleRootHashSize               = 16
	VblakePreviousBlockHashSize         = 12
	VblakePreviousKeystoneHashSize      = 9
	VbkHeaderSizeVblake                 = 64
	VbkHeaderSizeProgpow                = 65
	MaxLayerCountMerkle                 = 40
	//! absolute maximum number of pauouts per ALT block
	MaxPayout = 50000
	//! NodeCore is using byte value when serializing outputs so we limit to 255
	MaxOutputsCount = 255
	//! ASN.1/DER ECDSA encoding max value
	MaxSignatureSize = 72
	//! X509 encoding. Max value is based on experimental data.
	PublicKeySize       = 88
	AddressSize         = 30
	MultisigAddressSize = 30
	//! absolute maximum allowed size for altchain hash
	MinAltHashSize = 12
	MaxAltHashSize = 1024

	VbkMaximumDifficulty = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"

	MaxMerkleBytes = (Sha256HashSize+1)*MaxLayerCountMerkle + 5 + 5 + 5 + 5 + 4

	MaxSizePublicationData =
	// identifier.size, identifier
	9 +
		// header.size.size, header.size, header
		5 + MaxHeaderSizePublicationData +
		// payoutInfo.size.size, payoutInfo.size, payoutInfo
		5 + MaxPayoutSizePublicationData +
		// contextInfo.size.size, contextInfo.size, contextInfo
		5 + MaxContextSizePublicationData

	MaxRawtxSizeVbktx =
	// network byte, type
	1 + 1 +
		// sourceAddress.size, sourceAddress
		1 + AddressSize +
		// sourceAmount.size, sourceAmount
		1 + 8 +
		// outputs.size, outputs.size * (output.address.size + output.address +
		// output.amount.size + output.amount)
		1 + MaxOutputsCount*(1+AddressSize+1+8) +
		// signatureIndex.size, signatureIndex
		1 + 8 +
		// data.size.size, data.size, data
		5 + MaxSizePublicationData +
		// signature.size, signature
		1 + MaxSignatureSize +
		// publicKey.size, publicKey
		1 + PublicKeySize +
		// raw.size.size, raw.size
		5

	MaxRawtxSizeVbkpoptx =
	// network byte, type
	1 + 1 +
		// address.size, address
		1 + AddressSize +
		// publishedBlock.size, publishedBlock
		1 + VbkHeaderSizeProgpow +
		// bitcoinTransaction.size.size, bitcoinTransaction.size, bitcoinTransaction
		5 + BtcTxMaxRawSize + MaxMerkleBytes +
		// blockOfProof.size, blockOfProof
		1 + BtcHeaderSize +
		// blockOfProofContext.size.size, blockOfProofContext.size,
		// blockOfProofContext
		5 + (BtcHeaderSize+1)*MaxContextCount +
		// signature.size, signature
		1 + MaxSignatureSize +
		// publicKey.size, publicKey
		1 + PublicKeySize +
		// raw.size.size, raw.size
		5
)
