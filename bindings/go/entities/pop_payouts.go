// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/binary"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

type Payout struct {
	PayoutInfo []byte
	Amount     uint64
}

type PopPayouts struct {
	payouts []Payout
}

// GetPayouts ...
func (v *PopPayouts) GetPayouts() []Payout {
	return v.payouts
}

// ToVbkEncoding ...
func (v *PopPayouts) ToVbkEncoding(stream io.Writer) error {
	return veriblock.WriteArrayOf(stream, v.payouts, func(stream io.Writer, val interface{}) error {
		payout := val.(Payout)
		if err := veriblock.WriteSingleByteLenValue(stream, payout.PayoutInfo); err != nil {
			return err
		}
		if err := binary.Write(stream, binary.BigEndian, payout.Amount); err != nil {
			return err
		}
		return nil
	})
}

// ToVbkEncodingBytes ...
func (v *PopPayouts) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// FromVbkEncoding ...
func (v *PopPayouts) FromVbkEncoding(stream io.Reader) error {
	payouts, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxPayout, func(stream io.Reader) (interface{}, error) {
		var payout Payout
		var err error
		payout.PayoutInfo, err = veriblock.ReadSingleByteLenValue(stream, 0, veriblock.MaxPayoutSizePublicationData)
		if err != nil {
			return nil, err
		}
		if err = binary.Read(stream, binary.BigEndian, &payout.Amount); err != nil {
			return nil, err
		}

		return &payout, nil
	})
	if err != nil {
		return err
	}

	v.payouts = make([]Payout, len(payouts))
	for i, payout := range payouts {
		v.payouts[i] = *payout.(*Payout)
	}

	return nil
}

// FromVbkEncodingBytes ...
func (v *PopPayouts) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}
