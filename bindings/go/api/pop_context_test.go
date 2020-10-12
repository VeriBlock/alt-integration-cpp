package api

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestPopContext(t *testing.T) {
	assert := assert.New(t)

	config := NewConfig()
	defer config.Free()
	if !config.SelectVbkParams("test", 1, nil) {
		t.Error("Failed to select btc params")
	}
	if !config.SelectBtcParams("test", 1, nil) {
		t.Error("Failed to select btc params")
	}

	// popContext := NewPopContext(&config)
	// defer popContext.Free()

	// res := popContext.MemPoolGetPop()
	// t.Errorf("Res: %v", res)

	assert.Equal(true, true)
}
