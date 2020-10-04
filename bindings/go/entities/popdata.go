package entities

import (
	"io"
	"math"

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
	_, _ = veriblock.ReadArrayOf(r, 0, int64(math.MaxInt32), veriblock.ReadArrayOfFunc)
}
