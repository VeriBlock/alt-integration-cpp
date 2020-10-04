package entities

import (
	"bytes"
	"crypto/sha256"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// VbkPopTx ...
type VbkPopTx struct {
	NetworkOrType       veriblock.NetworkBytePair
	Address             Address
	PublishedBlock      VbkBlock
	BitcoinTransaction  BtcTx
	MerklePath          MerklePath
	BlockOfProof        BtcBlock
	BlockOfProofContext []BtcBlock
	Signature           []byte
	PublicKey           []byte
}

// ToVbkEncoding ...
func (v *VbkPopTx) ToVbkEncoding(stream io.Writer) error {
	txStream := new(bytes.Buffer)
	err := v.ToRaw(txStream)
	if err != nil {
		return err
	}
	err = veriblock.WriteVarLenValue(stream, txStream.Bytes())
	if err != nil {
		return err
	}
	err = veriblock.WriteSingleByteLenValue(stream, v.Signature)
	if err != nil {
		return err
	}
	return veriblock.WriteSingleByteLenValue(stream, v.PublicKey)
}

// ToRaw ...
func (v *VbkPopTx) ToRaw(stream io.Writer) error {
	err := v.NetworkOrType.Write(stream)
	if err != nil {
		return err
	}
	err = v.Address.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = v.PublishedBlock.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = v.BitcoinTransaction.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = v.MerklePath.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = v.BlockOfProof.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = veriblock.WriteSingleBEValue(stream, int64(len(v.BlockOfProofContext)))
	if err != nil {
		return err
	}
	for _, block := range v.BlockOfProofContext {
		err = block.ToVbkEncoding(stream)
		if err != nil {
			return err
		}
	}
	return nil
}

// VbkPopTxFromVbkEncoding ...
func VbkPopTxFromVbkEncoding(stream io.Reader) (*VbkPopTx, error) {
	rawTx, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxRawtxSizeVbkpoptx)
	if err != nil {
		return nil, err
	}
	signature, err := veriblock.ReadSingleByteLenValue(stream, 0, veriblock.MaxSignatureSize)
	if err != nil {
		return nil, err
	}
	publicKey, err := veriblock.ReadSingleByteLenValue(stream, 0, veriblock.PublicKeySize)
	if err != nil {
		return nil, err
	}
	rawTxStream := bytes.NewReader(rawTx)
	return VbkPopTxFromRaw(rawTxStream, signature, publicKey)
}

// VbkPopTxFromRaw ...
func VbkPopTxFromRaw(stream io.Reader, signature []byte, publicKey []byte) (*VbkPopTx, error) {
	networkOrType, err := veriblock.ReadNetworkByte(stream, veriblock.TxTypeVbkPopTx)
	if err != nil {
		return nil, err
	}
	address, err := AddressFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	publishedBlock, err := VbkBlockFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	bitcoinTransaction, err := BtcTxFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	hashFirst := sha256.Sum256(bitcoinTransaction.Tx)
	hash := sha256.Sum256(hashFirst[:])
	merklePath, err := MerklePathFromVbkEncoding(stream, hash)
	if err != nil {
		return nil, err
	}
	blockOfProof, err := BtcBlockFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	bopcs, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxContextCount, func(stream io.Reader) (interface{}, error) {
		return BtcBlockFromVbkEncoding(stream)
	})
	if err != nil {
		return nil, err
	}
	blockOfProofContext := make([]BtcBlock, len(bopcs))
	for i, bopc := range bopcs {
		blockOfProofContext[i] = *bopc.(*BtcBlock)
	}
	tx := VbkPopTx{
		*networkOrType,
		*address,
		*publishedBlock,
		*bitcoinTransaction,
		*merklePath,
		*blockOfProof,
		blockOfProofContext,
		signature,
		publicKey,
	}
	return &tx, nil
}
