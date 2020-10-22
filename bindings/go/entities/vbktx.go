package entities

import (
	"bytes"
	"encoding/binary"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// VbkTx ...
type VbkTx struct {
	NetworkOrType   veriblock.NetworkBytePair
	SourceAddress   Address
	SourceAmount    Coin
	Outputs         []Output
	SignatureIndex  int64
	PublicationData PublicationData
	Signature       []byte
	PublicKey       []byte
}

// ToVbkEncoding ...
func (v *VbkTx) ToVbkEncoding(stream io.Writer) error {
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
func (v *VbkTx) ToRaw(stream io.Writer) error {
	if err := v.NetworkOrType.Write(stream); err != nil {
		return err
	}
	if err := v.SourceAddress.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.SourceAmount.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.BigEndian, uint8(len(v.Outputs))); err != nil {
		return err
	}
	for _, output := range v.Outputs {
		if err := output.ToVbkEncoding(stream); err != nil {
			return err
		}
	}
	if err := veriblock.WriteSingleBEValue(stream, v.SignatureIndex); err != nil {
		return err
	}
	pubBytesStream := new(bytes.Buffer)
	if err := v.PublicationData.ToRaw(pubBytesStream); err != nil {
		return err
	}
	return veriblock.WriteVarLenValue(stream, pubBytesStream.Bytes())
}

// FromVbkEncoding ...
func (v *VbkTx) FromVbkEncoding(stream io.Reader) error {
	rawTx, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxRawtxSizeVbktx)
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
func (v *VbkTx) FromRaw(stream io.Reader, signature []byte, publicKey []byte) error {
	if err := v.NetworkOrType.Read(stream, veriblock.TxTypeVbkTx); err != nil {
		return err
	}
	if err := v.SourceAddress.FromVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.SourceAmount.FromVbkEncoding(stream); err != nil {
		return err
	}
	var outputSize uint8
	if err := binary.Read(stream, binary.BigEndian, &outputSize); err != nil {
		return err
	}
	v.Outputs = make([]Output, outputSize)
	for i := 0; i < int(outputSize); i++ {
		if err := v.Outputs[i].FromVbkEncoding(stream); err != nil {
			return err
		}
	}
	if err := veriblock.ReadSingleBEValue(stream, &v.SignatureIndex); err != nil {
		return err
	}
	pubBytes, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxSizePublicationData)
	if err != nil {
		return err
	}
	pubBytesStream := bytes.NewReader(pubBytes)
	if err := v.PublicationData.FromRaw(pubBytesStream); err != nil {
		return err
	}
	v.Signature = signature
	v.PublicKey = publicKey
	return nil
}
