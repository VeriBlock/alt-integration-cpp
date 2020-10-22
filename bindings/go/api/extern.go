package api

import (
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
func SetOnGetAtv(fn func(id []byte) []byte) {
	ffi.OnGetAtv = fn
}

// SetOnGetVtb ...
func SetOnGetVtb(fn func(id []byte) []byte) {
	ffi.OnGetVtb = fn
}

// SetOnGetVbk ...
func SetOnGetVbk(fn func(id []byte) []byte) {
	ffi.OnGetVbk = fn
}
