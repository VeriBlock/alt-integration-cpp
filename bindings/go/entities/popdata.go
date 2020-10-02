package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// PopData ...
type PopData struct {
	version uint32
	context []VbkBlock
	// vtbs    []VTB
	// atvs    []ATV
}

// NewPopData ...
func NewPopData(context []VbkBlock) PopData {
	return PopData{1, context}
}

// FromRaw ...
func (v PopData) FromRaw(r io.Reader) {
	_, _ = veriblock.ReadArrayOf(r, veriblock.ReadArrayOfFunc)
}
