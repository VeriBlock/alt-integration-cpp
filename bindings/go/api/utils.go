package api

import (
	"errors"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
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

func (v *PopContext) checkATV(atv *entities.Atv) error {
	defer v.lock()()

	bytes, err := atv.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	ok := v.popContext.CheckATV(bytes)
	if !ok {
		return errors.New("ATV stateless invalid")
	}

	return nil
}

func (v *PopContext) checkVTB(vtb *entities.Vtb) error {
	defer v.lock()()

	bytes, err := vtb.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	ok := v.popContext.CheckVTB(bytes)
	if !ok {
		return errors.New("VTB stateless invalid")
	}
	return nil
}

func (v *PopContext) checkVbkBlock(blk *entities.VbkBlock) error {
	defer v.lock()()

	bytes, err := blk.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	ok := v.popContext.CheckVbkBlock(bytes)
	if !ok {
		return errors.New("VbkBlock stateless invalid")
	}
	return nil
}

func (v *PopContext) checkPopData(popData *entities.PopData) error {
	defer v.lock()()

	bytes, err := popData.ToVbkEncodingBytes()
	if err != nil {
		return err
	}
	ok := v.popContext.CheckPopData(bytes)
	if !ok {
		return errors.New("PopData stateless invalid")
	}
	return nil
}
