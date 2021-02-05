package api

import ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"

type Storage struct {
	*ffi.Storage
}

func NewStorage(path string) *Storage {
	cfg := ffi.NewStorage(path)
	return &Storage{cfg}
}
