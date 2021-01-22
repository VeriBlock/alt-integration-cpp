package api

import (
	"bytes"
	"fmt"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestCalculateTopLevelMerkleRoot(t *testing.T) {
	assert := assert.New(t)

	fmt.Println("TestCalculateTopLevelMerkleRoot run")
	popContext := generateTestPopContext(t)
	defer popContext.popContext.Free()

	index, err := popContext.AltBestBlock()
	assert.NoError(err)

	block, err := index.GetAltBlockHeader()
	assert.NoError(err)

	var popData entities.PopData
	popData.Version = 1

	txRootHash := [veriblock.Sha256HashSize]byte{}

	hash, err := popContext.CalculateTopLevelMerkleRoot(txRootHash, block.PreviousBlock, &popData)
	assert.NoError(err)

	assert.False(bytes.Equal(hash[:], []byte{}))
}
