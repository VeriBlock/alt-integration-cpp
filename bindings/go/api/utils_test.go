package api

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestCalculateContextInfoContainerHash(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)
	defer popContext.Free()

	index, err := popContext.AltBestBlock()
	assert.NoError(err)

	block, err := index.GetAltBlockHeader()
	assert.NoError(err)

	var popData entities.PopData
	popData.Version = 1

	hash, err := popContext.CalculateContextInfoContainerHash(block, &popData)
	assert.NoError(err)
	fmt.Println(hex.EncodeToString(hash[:]))

	assert.False(bytes.Equal(hash[:], []byte{}))
}
