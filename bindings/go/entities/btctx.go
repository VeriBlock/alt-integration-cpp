package entities

import (
	"bytes"
	"encoding/hex"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// BtcTx ...
type BtcTx struct {
	Tx []byte
}

// ToVbkEncoding ...
func (v *BtcTx) ToVbkEncoding(stream io.Writer) error {
	return veriblock.WriteVarLenValue(stream, v.Tx)
}

// ToVbkEncodingBytes ...
func (v *BtcTx) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// FromVbkEncoding ...
func (v *BtcTx) FromVbkEncoding(stream io.Reader) error {
	tx, err := veriblock.ReadVarLenValue(stream, 0, veriblock.BtcTxMaxRawSize)
	if err != nil {
		return err
	}
	v.Tx = tx
	return nil
}

// FromVbkEncodingBytes ...
func (v *BtcTx) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}

// ToJSON ...
func (v *BtcTx) ToJSON() (string, error) {
	return hex.EncodeToString(v.Tx), nil
}
