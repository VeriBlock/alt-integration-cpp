package entities

import (
	"bytes"
	"io"
)

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

// ToVbkEncodingBytes ...
func (v *Output) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// FromVbkEncoding ...
func (v *Output) FromVbkEncoding(stream io.Reader) error {
	if err := v.Address.FromVbkEncoding(stream); err != nil {
		return err
	}
	return v.Coin.FromVbkEncoding(stream)
}

// FromVbkEncodingBytes ...
func (v *Output) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}

// ToJSON ...
func (v *Output) ToJSON() (map[string]interface{}, error) {
	res := map[string]interface{}{
		"address": v.Address.ToString(),
		"coin":    v.Coin.Units,
	}
	return res, nil
}
