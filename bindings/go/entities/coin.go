// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
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

// ToVbkEncodingBytes ...
func (v *Coin) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// FromVbkEncoding ...
func (v *Coin) FromVbkEncoding(stream io.Reader) error {
	return veriblock.ReadSingleBEValue(stream, &v.Units)
}

// FromVbkEncodingBytes ...
func (v *Coin) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}
