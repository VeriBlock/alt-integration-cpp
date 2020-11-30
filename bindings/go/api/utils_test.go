package api

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestCalculateContextInfoContainerHash(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)
	defer popContext.Free()

	_, err := popContext.AltBestBlock()
	assert.NoError(err)

	// _, err = index.GetAltBlockHeader()
	// assert.NoError(err)

}
