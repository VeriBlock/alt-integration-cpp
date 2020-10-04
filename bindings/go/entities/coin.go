package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// Coin ...
type Coin struct {
	Units int64
}

// ToVbkEncoding ...
func (v *Coin) ToVbkEncoding(stream io.Writer) error {
	return veriblock.WriteSingleBEValue(stream, v.Units)
}

// CoinFromVbkEncoding ...
func CoinFromVbkEncoding(stream io.Reader) (*Coin, error) {
	coin := Coin{}
	err := veriblock.ReadSingleBEValue(stream, &coin.Units)
	if err != nil {
		return nil, err
	}
	return &coin, nil
}
