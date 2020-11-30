package api

import (
	"bytes"
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

	var buffer bytes.Buffer
	err := popData.ToVbkEncoding(&buffer)
	if err == nil {
		return nil, err
	}

	var hash entities.ContextInfoContainerHash
	hash = v.popContext.AltBlockCalculateContextInfoContainerHash(prevAltBlock.Hash, buffer.Bytes())
	return &hash, nil
}
