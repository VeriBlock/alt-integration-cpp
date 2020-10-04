package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
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
	err := veriblock.WriteSingleBEValue(stream, v.Identifier)
	if err != nil {
		return err
	}
	err = veriblock.WriteVarLenValue(stream, v.Header)
	if err != nil {
		return err
	}
	err = veriblock.WriteVarLenValue(stream, v.ContextInfo)
	if err != nil {
		return err
	}
	return veriblock.WriteVarLenValue(stream, v.PayoutInfo)
}

// PublicationDataFromRaw ...
func PublicationDataFromRaw(stream io.Reader) (*PublicationData, error) {
	data := PublicationData{}
	err := veriblock.ReadSingleBEValue(stream, &data.Identifier)
	if err != nil {
		return nil, err
	}
	data.Header, err = veriblock.ReadVarLenValue(stream, 0, veriblock.MaxHeaderSizePublicationData)
	if err != nil {
		return nil, err
	}
	data.ContextInfo, err = veriblock.ReadVarLenValue(stream, 0, veriblock.MaxContextSizePublicationData)
	if err != nil {
		return nil, err
	}
	data.PayoutInfo, err = veriblock.ReadVarLenValue(stream, 0, veriblock.MaxPayoutSizePublicationData)
	if err != nil {
		return nil, err
	}
	return &data, nil
}
