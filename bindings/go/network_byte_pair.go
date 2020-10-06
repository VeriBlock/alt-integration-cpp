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
		if err := binary.Write(stream, binary.BigEndian, v.NetworkByte); err != nil {
			return err
		}
	}
	return binary.Write(stream, binary.BigEndian, v.TypeID)
}

// Read ...
func (v *NetworkBytePair) Read(stream io.Reader, txType TxType) error {
	var networkOrType uint8
	if err := binary.Read(stream, binary.BigEndian, &networkOrType); err != nil {
		return err
	}
	if networkOrType == uint8(txType) {
		v.TypeID = networkOrType
		return nil
	}
	v.HasNetworkByte = true
	v.NetworkByte = networkOrType
	return binary.Read(stream, binary.BigEndian, &v.TypeID)
}
