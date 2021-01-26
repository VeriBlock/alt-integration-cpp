package api

import (
	"errors"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

func (v *PopContext) GeneratePublicationData(endorsedBlockHeader []byte, txRootHash [veriblock.Sha256HashSize]byte, popData *entities.PopData, payoutInfo []byte) (*entities.PublicationData, error) {
	defer v.lock()()

	popDataBytes, err := popData.ToVbkEncodingBytes()
	if err != nil {
		return nil, err
	}

	stream := v.popContext.AltBlockGeneratePublicationData(endorsedBlockHeader, txRootHash, popDataBytes, payoutInfo)
	if stream == nil {
		return nil, errors.New("cannot generate PublicationData")
	}
	defer stream.Free()

	publicationData := &entities.PublicationData{}
	publicationData.FromVbkEncoding(stream)

	return publicationData, nil
}

func (v *PopContext) CalculateTopLevelMerkleRoot(txRootHash [veriblock.Sha256HashSize]byte, prevAltBlockHash entities.AltHash, popData *entities.PopData) (*entities.ContextInfoContainerHash, error) {
	defer v.lock()()
	if popData == nil {
		return nil, errors.New("popData should be defined")
	}

	popDataBytes, err := popData.ToVbkEncodingBytes()
	if err != nil {
		return nil, err
	}

	var hash entities.ContextInfoContainerHash
	hash = v.popContext.AltBlockCalculateTopLevelMerkleRoot(txRootHash, prevAltBlockHash, popDataBytes)
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

func (v *PopContext) SaveAllTrees() error {
	defer v.lock()()

	state := ffi.NewValidationState()
	defer state.Free()

	ok := v.popContext.SaveAllTrees(state)
	if !ok {
		return state.Error()
	}
	return nil
}

func (v *PopContext) LoadAllTrees() error {
	defer v.lock()()

	state := ffi.NewValidationState()
	defer state.Free()

	ok := v.popContext.LoadAllTrees(state)
	if !ok {
		return state.Error()
	}
	return nil
}
