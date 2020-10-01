package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/src"
)

// PopData ...
type PopData struct{}

// FromRaw ...
func (v PopData) FromRaw(r io.Reader) {
	_, _ = veriblock.ReadArrayOf(r, veriblock.ReadArrayOfFunc)
}
