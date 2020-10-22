package entities

import (
	"bytes"
	"crypto/sha256"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
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
	if err := v.ToRaw(txStream); err != nil {
		return err
	}
	if err := veriblock.WriteVarLenValue(stream, txStream.Bytes()); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.Signature); err != nil {
		return err
	}
	return veriblock.WriteSingleByteLenValue(stream, v.PublicKey)
}

// ToRaw ...
func (v *VbkPopTx) ToRaw(stream io.Writer) error {
	if err := v.NetworkOrType.Write(stream); err != nil {
		return err
	}
	if err := v.Address.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.PublishedBlock.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.BitcoinTransaction.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.MerklePath.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.BlockOfProof.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := veriblock.WriteSingleBEValue(stream, int64(len(v.BlockOfProofContext))); err != nil {
		return err
	}
	for _, block := range v.BlockOfProofContext {
		if err := block.ToVbkEncoding(stream); err != nil {
			return err
		}
	}
	return nil
}

// FromVbkEncoding ...
func (v *VbkPopTx) FromVbkEncoding(stream io.Reader) error {
	rawTx, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxRawtxSizeVbkpoptx)
	if err != nil {
		return err
	}
	signature, err := veriblock.ReadSingleByteLenValue(stream, 0, veriblock.MaxSignatureSize)
	if err != nil {
		return err
	}
	publicKey, err := veriblock.ReadSingleByteLenValue(stream, 0, veriblock.PublicKeySize)
	if err != nil {
		return err
	}
	rawTxStream := bytes.NewReader(rawTx)
	return v.FromRaw(rawTxStream, signature, publicKey)
}

// FromRaw ...
func (v *VbkPopTx) FromRaw(stream io.Reader, signature []byte, publicKey []byte) error {
	if err := v.NetworkOrType.Read(stream, veriblock.TxTypeVbkPopTx); err != nil {
		return err
	}
	if err := v.Address.FromVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.PublishedBlock.FromVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.BitcoinTransaction.FromVbkEncoding(stream); err != nil {
		return err
	}
	hashFirst := sha256.Sum256(v.BitcoinTransaction.Tx)
	hash := sha256.Sum256(hashFirst[:])
	if err := v.MerklePath.FromVbkEncoding(stream, hash); err != nil {
		return err
	}
	if err := v.BlockOfProof.FromVbkEncoding(stream); err != nil {
		return err
	}
	bopcs, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxContextCount, func(stream io.Reader) (interface{}, error) {
		block := BtcBlock{}
		err := block.FromVbkEncoding(stream)
		if err != nil {
			return nil, err
		}
		return &block, nil
	})
	if err != nil {
		return err
	}
	v.BlockOfProofContext = make([]BtcBlock, len(bopcs))
	for i, bopc := range bopcs {
		v.BlockOfProofContext[i] = *bopc.(*BtcBlock)
	}
	v.Signature = signature
	v.PublicKey = publicKey
	return nil
}
