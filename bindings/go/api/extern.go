package api

import (
	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
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

// SetOnIsHeader ...
func SetOnIsHeader(fn func(data []byte) bool) {
	ffi.OnIsHeader = fn
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
