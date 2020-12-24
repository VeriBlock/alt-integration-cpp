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

// SetOnGetAtv ...
func SetOnGetAtv(fn func(idBytes entities.AtvID) []byte) {
	ffi.OnGetAtv = func(id []byte) []byte {
		return fn(entities.ParseAtvID(id))
	}
}

// SetOnGetVtb ...
func SetOnGetVtb(fn func(id entities.VtbID) []byte) {
	ffi.OnGetVtb = func(id []byte) []byte {
		return fn(entities.ParseVtbID(id))
	}
}

// SetOnGetVbk ...
func SetOnGetVbk(fn func(id entities.VbkID) []byte) {
	ffi.OnGetVbk = func(id []byte) []byte {
		return fn(entities.ParseVbkID(id))
	}
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
