package api

import (
	ffi "github.com/VeriBlock/alt-integration-cpp/ffi"
)

// Config - Configuration for Veriblock integration
type Config struct {
	*ffi.Config
}

// NewConfig - Returns instance of Config struct. Allocates memory
func NewConfig() Config {
	cfg := ffi.NewConfig()
	return Config{&cfg}
}
