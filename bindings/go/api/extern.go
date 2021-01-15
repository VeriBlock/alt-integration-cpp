package api

import (
	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

type onAcceptedATVSignature func(atv *entities.Atv)
type onAcceptedVTBSignature func(atv *entities.Vtb)
type onAcceptedVBKSignature func(atv *entities.VbkBlock)

var (
	onAcceptedATV = []onAcceptedATVSignature{}
	onAcceptedVTB = []onAcceptedVTBSignature{}
	onAcceptedVBK = []onAcceptedVBKSignature{}
)

// SetOnGetAltchainID ...
func SetOnGetAltchainID(fn func() int64) {
	ffi.OnGetAltchainID = fn
}

// SetOnGetBootstrapBlock ...
func SetOnGetBootstrapBlock(fn func() string) {
	ffi.OnGetBootstrapBlock = fn
}

// SetOnGetBlockHeaderHash ...
func SetOnGetBlockHeaderHash(fn func(toBeHashed []byte) []byte) {
	ffi.OnGetBlockHeaderHash = fn
}

// SetOnCheckBlockHeader ...
func SetOnCheckBlockHeader(fn func(header []byte, root []byte) bool) {
	ffi.OnCheckBlockHeader = fn
}

// AddOnAcceptedATV ...
func (v *PopContext) AddOnAcceptedATV(fn onAcceptedATVSignature) {
	defer v.lock()()
	onAcceptedATV = append(onAcceptedATV, fn)
}

// SetOnAcceptedVTB ...
func (v *PopContext) AddOnAcceptedVTB(fn onAcceptedVTBSignature) {
	defer v.lock()()
	onAcceptedVTB = append(onAcceptedVTB, fn)
}

// AddOnAcceptedVBK ...
func (v *PopContext) AddOnAcceptedVBK(fn onAcceptedVBKSignature) {
	defer v.lock()()
	onAcceptedVBK = append(onAcceptedVBK, fn)
}
