package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// Coin ...
type Coin struct {
	Units int64
}

// ToVbkEncoding ...
func (v *Coin) ToVbkEncoding(stream io.Writer) error {
	return veriblock.WriteSingleBEValue(stream, v.Units)
}

// FromVbkEncoding ...
func (v *Coin) FromVbkEncoding(stream io.Reader) error {
	return veriblock.ReadSingleBEValue(stream, &v.Units)
}
