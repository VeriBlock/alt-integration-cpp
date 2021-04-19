// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
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

// GetHash ...
func (v *VbkTx) GetHash() []byte {
	// TODO: Add hash of transaction
	res := make([]byte, 32)
	return res
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

// ToVbkEncodingBytes ...
func (v *VbkTx) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
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
	if err := v.PublicationData.ToVbkEncoding(pubBytesStream); err != nil {
		return err
	}
	return veriblock.WriteVarLenValue(stream, pubBytesStream.Bytes())
}

// ToRawBytes ...
func (v *VbkTx) ToRawBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToRaw(&buffer)
	return buffer.Bytes(), err
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

// FromVbkEncodingBytes ...
func (v *VbkTx) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
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
	if err := v.PublicationData.FromVbkEncoding(pubBytesStream); err != nil {
		return err
	}
	v.Signature = signature
	v.PublicKey = publicKey
	return nil
}

// FromRawBytes ...
func (v *VbkTx) FromRawBytes(data []byte, signature []byte, publicKey []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromRaw(buffer, signature, publicKey)
}

// ToJSON ...
func (v *VbkTx) ToJSON() (map[string]interface{}, error) {
	outputs := make([]interface{}, len(v.Outputs))
	for i, output := range v.Outputs {
		out, err := output.ToJSON()
		if err != nil {
			return nil, err
		}
		outputs[i] = out
	}
	publicationData, err := v.PublicationData.ToJSON()
	if err != nil {
		return nil, err
	}
	res := map[string]interface{}{
		"hash":            hex.EncodeToString(v.GetHash()),
		"networkByte":     v.NetworkOrType.NetworkByte,
		"type":            v.NetworkOrType.TypeID,
		"sourceAddress":   v.SourceAddress.ToString(),
		"sourceAmount":    v.SourceAmount.Units,
		"outputs":         outputs,
		"signatureIndex":  v.SignatureIndex,
		"publicationData": publicationData,
		"signature":       hex.EncodeToString(v.Signature),
		"publicKey":       hex.EncodeToString(v.PublicKey),
	}
	return res, nil
}
