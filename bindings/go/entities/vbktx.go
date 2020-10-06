package entities

import (
	"bytes"
	"encoding/binary"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
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
func (v *VbkTx) ToRaw(stream io.Writer) error {
	err := v.NetworkOrType.Write(stream)
	if err != nil {
		return err
	}
	err = v.SourceAddress.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = v.SourceAmount.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = binary.Write(stream, binary.BigEndian, uint8(len(v.Outputs)))
	if err != nil {
		return err
	}
	for _, output := range v.Outputs {
		err = output.ToVbkEncoding(stream)
		if err != nil {
			return err
		}
	}
	err = veriblock.WriteSingleBEValue(stream, v.SignatureIndex)
	if err != nil {
		return err
	}
	pubBytesStream := new(bytes.Buffer)
	err = v.PublicationData.ToRaw(pubBytesStream)
	if err != nil {
		return err
	}
	return veriblock.WriteVarLenValue(stream, pubBytesStream.Bytes())
}

// VbkTxFromVbkEncoding ...
func VbkTxFromVbkEncoding(stream io.Reader) (*VbkTx, error) {
	rawTx, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxRawtxSizeVbktx)
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
	return VbkTxFromRaw(rawTxStream, signature, publicKey)
}

// VbkTxFromRaw ...
func VbkTxFromRaw(stream io.Reader, signature []byte, publicKey []byte) (*VbkTx, error) {
	networkOrType, err := veriblock.ReadNetworkByte(stream, veriblock.TxTypeVbkTx)
	if err != nil {
		return nil, err
	}
	sourceAddress, err := AddressFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	sourceAmount, err := CoinFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	var outputSize uint8
	err = binary.Read(stream, binary.BigEndian, &outputSize)
	outputs := make([]Output, outputSize)
	for i := 0; i < int(outputSize); i++ {
		output, err := OutputFromVbkEncoding(stream)
		if err != nil {
			return nil, err
		}
		outputs[i] = *output
	}
	var signatureIndex int64
	err = veriblock.ReadSingleBEValue(stream, &signatureIndex)
	if err != nil {
		return nil, err
	}
	pubBytes, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxSizePublicationData)
	if err != nil {
		return nil, err
	}
	pubBytesStream := bytes.NewReader(pubBytes)
	publicationData, err := PublicationDataFromRaw(pubBytesStream)
	if err != nil {
		return nil, err
	}
	tx := VbkTx{
		*networkOrType,
		*sourceAddress,
		*sourceAmount,
		outputs,
		signatureIndex,
		*publicationData,
		signature,
		publicKey,
	}
	return &tx, nil
}
