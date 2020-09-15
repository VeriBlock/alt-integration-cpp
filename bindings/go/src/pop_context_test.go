package veriblock

import "testing"

func TestPopContext(t *testing.T) {
	config := NewConfig()
	defer config.Free()

	if !config.SelectVbkParams("test", popvbkstartheight, &popvbkblocks) {
		t.Error("Failed to select btc params")
	}
	if !config.SelectBtcParams("test", popbtcstartheight, &popbtcblocks) {
		t.Error("Failed to select btc params")
	}

	// popContext := NewPopContext(&config)
	// defer popContext.Free()

	// res := popContext.MemPoolGetPop()
	// t.Errorf("Res: %v", res)
}
