// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"errors"
	"io"
	"math"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

func (v *PopContext) AltBlockGetEndorsedBy(altblockHash []byte) ([]entities.AltEndorsement, error) {
	stream := v.popContext.AltBlockGetEndorsedBy(altblockHash)
	if stream == nil {
		return nil, errors.New("cannot find alt block")
	}
	defer stream.Free()
	endorsements, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(stream io.Reader) (interface{}, error) {
		endorsement := entities.AltEndorsement{}
		err := endorsement.FromVbkEncoding(stream)
		if err != nil {
			return nil, err
		}
		return endorsement, nil
	})
	if err != nil {
		return nil, errors.New("failed to deserialize alt endorsement")
	}
	ends := make([]entities.AltEndorsement, len(endorsements))
	for i, endorsement := range endorsements {
		ends[i] = endorsement.(entities.AltEndorsement)
	}
	return ends, nil
}

func (v *PopContext) GeneratePublicationData(endorsedBlockHeader []byte, txRootHash [veriblock.Sha256HashSize]byte, popData *entities.PopData, payoutInfo []byte) (*entities.PublicationData, error) {
	if popData == nil {
		popData = entities.GetEmptyPopData()
	}

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	if popData == nil {
		return nil
	}
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
	v.mutex.AssertMutexLocked("pop context is not locked")

	state := ffi.NewValidationState()
	defer state.Free()

	ok := v.popContext.SaveAllTrees(state)
	if !ok {
		return state.Error()
	}
	return nil
}

func (v *PopContext) LoadAllTrees() error {
	v.mutex.AssertMutexLocked("pop context is not locked")

	state := ffi.NewValidationState()
	defer state.Free()

	ok := v.popContext.LoadAllTrees(state)
	if !ok {
		return state.Error()
	}
	return nil
}
