package api

import (
	"errors"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

func (v *PopContext) CalculateContextInfoContainerHash(prevAltBlockHash entities.AltHash, popData *entities.PopData) (*entities.ContextInfoContainerHash, error) {
	defer v.lock()()
	if popData == nil {
		return nil, errors.New("popData should be defined")
	}

	popDataBytes, err := popData.ToVbkEncodingBytes()
	if err != nil {
		return nil, err
	}

	var hash entities.ContextInfoContainerHash
	hash = v.popContext.AltBlockCalculateContextInfoContainerHash(prevAltBlockHash, popDataBytes)
	return &hash, nil
}

func (v *PopContext) CheckATV(atv *entities.Atv) error {
	defer v.lock()()
	bytes, err := atv.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	ok := v.popContext.CheckATV(bytes, state)
	if !ok {
		return state.Error()
	}

	return nil
}

func (v *PopContext) CheckVTB(vtb *entities.Vtb) error {
	defer v.lock()()
	bytes, err := vtb.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	ok := v.popContext.CheckVTB(bytes, state)
	if !ok {
		return state.Error()
	}
	return nil
}

func (v *PopContext) CheckVbkBlock(blk *entities.VbkBlock) error {
	defer v.lock()()
	bytes, err := blk.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	ok := v.popContext.CheckVbkBlock(bytes, state)
	if !ok {
		return state.Error()
	}
	return nil
}

func (v *PopContext) CheckPopData(popData *entities.PopData) error {
	defer v.lock()()
	bytes, err := popData.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	ok := v.popContext.CheckPopData(bytes, state)
	if !ok {
		return state.Error()
	}
	return nil
}
