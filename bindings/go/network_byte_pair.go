package veriblock

import (
	"encoding/binary"
	"io"
)

// NetworkBytePair ...
type NetworkBytePair struct {
	// If hasNetworkByte is true, networkByte is set.
	HasNetworkByte bool
	NetworkByte    byte
	TypeID         uint8
}

// Write ...
func (v *NetworkBytePair) Write(stream io.Writer) error {
	if v.HasNetworkByte {
		err := binary.Write(stream, binary.BigEndian, v.NetworkByte)
		if err != nil {
			return err
		}
	}
	return binary.Write(stream, binary.BigEndian, v.TypeID)
}

// ReadNetworkByte ...
func ReadNetworkByte(stream io.Reader, txType TxType) (*NetworkBytePair, error) {
	var networkOrType uint8
	err := binary.Read(stream, binary.BigEndian, &networkOrType)
	if err != nil {
		return nil, err
	}
	ret := NetworkBytePair{}
	if networkOrType == uint8(txType) {
		ret.TypeID = networkOrType
	} else {
		ret.HasNetworkByte = true
		ret.NetworkByte = networkOrType
		err := binary.Read(stream, binary.BigEndian, &ret.TypeID)
		if err != nil {
			return nil, err
		}
	}
	return &ret, nil
}
