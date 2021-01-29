package entities

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// PopData ...
type PopData struct {
	Version uint32
	Context []VbkBlock
	Vtbs    []Vtb
	Atvs    []Atv
}

func GetEmptyPopData() *PopData {
	return &PopData{Version: 1}
}

// ToVbkEncoding ...
func (v *PopData) ToVbkEncoding(stream io.Writer) error {
	if err := binary.Write(stream, binary.BigEndian, v.Version); err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("PopData serialization version=%d is not implemented", v.Version)
	}
	if err := veriblock.WriteSingleBEValue(stream, int64(len(v.Context))); err != nil {
		return err
	}
	for _, ctx := range v.Context {
		if err := ctx.ToVbkEncoding(stream); err != nil {
			return err
		}
	}
	if err := veriblock.WriteSingleBEValue(stream, int64(len(v.Vtbs))); err != nil {
		return err
	}
	for _, vtb := range v.Vtbs {
		if err := vtb.ToVbkEncoding(stream); err != nil {
			return err
		}
	}

	if err := veriblock.WriteSingleBEValue(stream, int64(len(v.Atvs))); err != nil {
		return err
	}
	for _, atv := range v.Atvs {
		if err := atv.ToVbkEncoding(stream); err != nil {
			return err
		}
	}
	return nil
}

// ToVbkEncodingBytes ...
func (v *PopData) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// FromVbkEncoding ...
func (v *PopData) FromVbkEncoding(stream io.Reader) error {
	if err := binary.Read(stream, binary.BigEndian, &v.Version); err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("PopData serialization version=%d is not implemented", v.Version)
	}
	contexts, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxPopDataVbkBlock, func(stream io.Reader) (interface{}, error) {
		block := VbkBlock{}
		err := block.FromVbkEncoding(stream)
		if err != nil {
			return nil, err
		}
		return &block, nil
	})
	if err != nil {
		return err
	}
	v.Context = make([]VbkBlock, len(contexts))
	for i, context := range contexts {
		v.Context[i] = *context.(*VbkBlock)
	}
	vtbs, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxPopDataVtb, func(stream io.Reader) (interface{}, error) {
		vtb := Vtb{}
		err := vtb.FromVbkEncoding(stream)
		if err != nil {
			return nil, err
		}
		return &vtb, nil
	})
	if err != nil {
		return err
	}
	v.Vtbs = make([]Vtb, len(vtbs))
	for i, vtb := range vtbs {
		v.Vtbs[i] = *vtb.(*Vtb)
	}
	atvs, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxPopDataAtv, func(stream io.Reader) (interface{}, error) {
		atv := Atv{}
		err := atv.FromVbkEncoding(stream)
		if err != nil {
			return nil, err
		}
		return &atv, nil
	})
	if err != nil {
		return err
	}
	v.Atvs = make([]Atv, len(atvs))
	for i, atv := range atvs {
		v.Atvs[i] = *atv.(*Atv)
	}
	return nil
}

// FromVbkEncodingBytes ...
func (v *PopData) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}
