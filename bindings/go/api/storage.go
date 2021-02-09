package api

import ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"

type Storage struct {
	*ffi.Storage
}

func NewStorage(path string) (*Storage, error) {
	state := ffi.NewValidationState()
	defer state.Free()

	cfg := ffi.NewStorage(path, state)
	if state.IsInvalid() {
		return nil, state.Error()
	} else {
		return &Storage{cfg}, nil
	}
}
