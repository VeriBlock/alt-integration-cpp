package api

import (
	"errors"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
)

func (v *PopContext) CalculateContextInfoContainerHash(prevAltBlock *entities.AltBlock, popData *entities.PopData) (*entities.ContextInfoContainerHash, error) {
	defer v.lock()()

	if prevAltBlock == nil {
		return nil, errors.New("prevAltBlock should be defined")
	}

	if popData == nil {
		return nil, errors.New("popData should be defined")
	}

	popDataBytes, err := popData.ToVbkEncodingBytes()
	if err == nil {
		return nil, err
	}

	var hash entities.ContextInfoContainerHash
	hash = v.popContext.AltBlockCalculateContextInfoContainerHash(prevAltBlock.Hash, popDataBytes)
	return &hash, nil
}
