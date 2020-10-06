package entities

import "io"

// Output ...
type Output struct {
	Address Address
	Coin    Coin
}

// ToVbkEncoding ...
func (v *Output) ToVbkEncoding(stream io.Writer) error {
	err := v.Address.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	return v.Coin.ToVbkEncoding(stream)
}

// OutputFromVbkEncoding ...
func OutputFromVbkEncoding(stream io.Reader) (*Output, error) {
	address, err := AddressFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	coin, err := CoinFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &Output{*address, *coin}, nil
}
