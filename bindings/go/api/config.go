package api

import (
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// Config - Configuration for Veriblock integration
type Config struct {
	*ffi.Config
}

// NewConfig - Returns instance of Config struct. Allocates memory
func NewConfig() *Config {
	cfg := ffi.NewConfig()
	return &Config{cfg}
}

func (v *Config) AltGetBootstrapBlock() *entities.AltBlock {
	stream := v.Config.AltGetBootstrapBlock()
	defer stream.Free()
	var blk entities.AltBlock
	blk.FromVbkEncoding(stream)
	return &blk
}
