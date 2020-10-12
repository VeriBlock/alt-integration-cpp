package api

import (
	ffi "github.com/VeriBlock/alt-integration-cpp/ffi"
)

// SetOnGetAltchainID ...
func SetOnGetAltchainID(fn func() int) {
	ffi.OnGetAltchainID = fn
}

// SetOnGetBootstrapBlock ...
func SetOnGetBootstrapBlock(fn func() string) {
	ffi.OnGetBootstrapBlock = fn
}
