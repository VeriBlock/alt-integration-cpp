package api

import (
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

// SetOnCheckBlockHeader ...
func SetOnCheckBlockHeader(fn func(header []byte, root []byte, state *ffi.ValidationState) bool) {
	ffi.OnCheckBlockHeader = fn
}

// SetOnLog ...
func SetOnLog(fn func(log_lvl string, msg string)) {
	ffi.OnLog = fn
}
