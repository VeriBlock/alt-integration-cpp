package api

import (
	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// SetOnGetAltchainID ...
func SetOnGetAltchainID(fn func() int) {
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
		var idBytes entities.AtvID
		copy(idBytes[:], id)
		return fn(idBytes)
	}
}

// SetOnGetVtb ...
func SetOnGetVtb(fn func(id entities.VtbID) []byte) {
	ffi.OnGetVtb = func(id []byte) []byte {
		var idBytes entities.VtbID
		copy(idBytes[:], id)
		return fn(idBytes)
	}
}

// SetOnGetVbk ...
func SetOnGetVbk(fn func(id entities.VbkID) []byte) {
	ffi.OnGetVbk = func(id []byte) []byte {
		var idBytes entities.VbkID
		copy(idBytes[:], id)
		return fn(idBytes)
	}
}
