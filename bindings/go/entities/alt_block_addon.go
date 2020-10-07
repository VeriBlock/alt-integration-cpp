package entities

import (
	"io"
	"math"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// AltBlockAddon ...
type AltBlockAddon struct {
	popState PopState
	// List of changes introduced in this block
	AtvIDs      [][32]byte
	VtbIDs      [][32]byte
	VbkBlockIDs [][12]byte
}

// ToRaw ...
func (v *AltBlockAddon) ToRaw(stream io.Writer) error {
	if err := v.popState.ToRaw(stream); err != nil {
		return err
	}
	if err := veriblock.WriteArrayOf(stream, v.AtvIDs, veriblock.WriteSingleByteLenValue); err != nil {
		return err
	}
	if err := veriblock.WriteArrayOf(stream, v.VtbIDs, veriblock.WriteSingleByteLenValue); err != nil {
		return err
	}
	return veriblock.WriteArrayOf(stream, v.VbkBlockIDs, veriblock.WriteSingleByteLenValue)
}

// FromRaw ...
func (v *AltBlockAddon) FromRaw(stream io.Reader) error {
	readFunc := func(r io.Reader) (interface{}, error) { return veriblock.ReadSingleByteLenValueDefault(r) }
	v.popState.IsAlt = true
	if err := v.popState.FromRaw(stream); err != nil {
		return err
	}
	atvids, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, readFunc)
	if err != nil {
		return err
	}
	v.AtvIDs = make([][32]byte, len(atvids))
	for i, atvid := range atvids {
		copy(v.AtvIDs[i][:], atvid.([]byte))
	}
	vtbids, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, readFunc)
	if err != nil {
		return err
	}
	v.VtbIDs = make([][32]byte, len(vtbids))
	for i, vtbid := range vtbids {
		copy(v.VtbIDs[i][:], vtbid.([]byte))
	}
	vbkblockids, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, readFunc)
	if err != nil {
		return err
	}
	v.VbkBlockIDs = make([][12]byte, len(vbkblockids))
	for i, vbkblockid := range vbkblockids {
		copy(v.VbkBlockIDs[i][:], vbkblockid.([]byte))
	}
	return nil
}
