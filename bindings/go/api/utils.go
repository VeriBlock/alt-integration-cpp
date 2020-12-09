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
