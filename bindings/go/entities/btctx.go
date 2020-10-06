package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// BtcTx ...
type BtcTx struct {
	Tx []byte
}

// ToVbkEncoding ...
func (v *BtcTx) ToVbkEncoding(stream io.Writer) error {
	return veriblock.WriteVarLenValue(stream, v.Tx)
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
