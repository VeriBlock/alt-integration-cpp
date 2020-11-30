package api

import (
	"bytes"
	"errors"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

func (v *PopContext) CalculateContextInfoContainerHash(prevAltBlock *entities.AltBlock, popData *entities.PopData) (entities.ContextInfoContainerHash, error)
{
	defer v.lock()()
	
	if(prevAltBlock == nil) {
		return nil, errors.New("prevAltBlock should be defined")
	}

	if(popData == nil) {
		return nil, errors.New("popData should be defined")
	}

	var buffer bytes.Buffer
	popData.ToVbkEncoding(buffer)

	return v.popContext.AltBlockCalculateContextInfoContainerHash(prevAltBlock.Hash, buffer.Bytes()) , nil
}
