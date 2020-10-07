package entities

import (
	"encoding/binary"
	"fmt"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// PopData ...
type PopData struct {
	Version uint32
	Context []VbkBlock
	Vtbs    []Vtb
	Atvs    []Atv
}

// ToVbkEncoding ...
func (v *PopData) ToVbkEncoding(stream io.Writer) error {
	err := binary.Write(stream, binary.BigEndian, v.Version)
	if err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("PopData serialization version=%d is not implemented", v.Version)
	}
	err = veriblock.WriteSingleBEValue(stream, int64(len(v.Context)))
	if err != nil {
		return err
	}
	for _, ctx := range v.Context {
		err = ctx.ToVbkEncoding(stream)
		if err != nil {
			return err
		}
	}
	err = veriblock.WriteSingleBEValue(stream, int64(len(v.Atvs)))
	if err != nil {
		return err
	}
	for _, atv := range v.Atvs {
		err = atv.ToVbkEncoding(stream)
		if err != nil {
			return err
		}
	}
	err = veriblock.WriteSingleBEValue(stream, int64(len(v.Vtbs)))
	if err != nil {
		return err
	}
	for _, vtb := range v.Vtbs {
		err = vtb.ToVbkEncoding(stream)
		if err != nil {
			return err
		}
	}
	return nil
}

// PopDataFromVbkEncoding ...
func PopDataFromVbkEncoding(stream io.Reader) (*PopData, error) {
	data := PopData{}
	err := binary.Read(stream, binary.BigEndian, &data.Version)
	if err != nil {
		return nil, err
	}
	if data.Version != 1 {
		return nil, fmt.Errorf("PopData serialization version=%d is not implemented", data.Version)
	}
	contexts, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxContextCount, func(stream io.Reader) (interface{}, error) {
		return VbkBlockFromVbkEncoding(stream)
	})
	if err != nil {
		return nil, err
	}
	data.Context = make([]VbkBlock, len(contexts))
	for i, context := range contexts {
		data.Context[i] = *context.(*VbkBlock)
	}
	atvs, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxContextCountAltPublication, func(stream io.Reader) (interface{}, error) {
		return AtvFromVbkEncoding(stream)
	})
	if err != nil {
		return nil, err
	}
	data.Atvs = make([]Atv, len(atvs))
	for i, atv := range atvs {
		data.Atvs[i] = *atv.(*Atv)
	}
	vtbs, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxContextCountVbkPublication, func(stream io.Reader) (interface{}, error) {
		return VtbFromVbkEncoding(stream)
	})
	if err != nil {
		return nil, err
	}
	data.Vtbs = make([]Vtb, len(vtbs))
	for i, vtb := range vtbs {
		data.Vtbs[i] = *vtb.(*Vtb)
	}
	return &data, nil
}
