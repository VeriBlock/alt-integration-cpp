package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// PublicationData ...
type PublicationData struct {
	// Altchain network POP ID
	Identifier int64
	// Altchain block header
	Header []byte
	// Bitcoin script or POP payout address
	PayoutInfo []byte
	// TBD
	ContextInfo []byte
}

// ToRaw ...
func (v *PublicationData) ToRaw(stream io.Writer) error {
	if err := veriblock.WriteSingleBEValue(stream, v.Identifier); err != nil {
		return err
	}
	if err := veriblock.WriteVarLenValue(stream, v.Header); err != nil {
		return err
	}
	if err := veriblock.WriteVarLenValue(stream, v.ContextInfo); err != nil {
		return err
	}
	return veriblock.WriteVarLenValue(stream, v.PayoutInfo)
}

// FromRaw ...
func (v *PublicationData) FromRaw(stream io.Reader) error {
	err := veriblock.ReadSingleBEValue(stream, &v.Identifier)
	if err != nil {
		return err
	}
	v.Header, err = veriblock.ReadVarLenValue(stream, 0, veriblock.MaxHeaderSizePublicationData)
	if err != nil {
		return err
	}
	v.ContextInfo, err = veriblock.ReadVarLenValue(stream, 0, veriblock.MaxContextSizePublicationData)
	if err != nil {
		return err
	}
	v.PayoutInfo, err = veriblock.ReadVarLenValue(stream, 0, veriblock.MaxPayoutSizePublicationData)
	if err != nil {
		return err
	}
	return nil
}
