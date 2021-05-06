// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

var defaultAltBlock = AltBlock{
	[]byte{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14},
	[]byte{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
	156,
	1466,
}

func TestAltBlockRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	outputStream := new(bytes.Buffer)
	assert.NoError(defaultAltBlock.ToVbkEncoding(outputStream))
	stream := bytes.NewReader(outputStream.Bytes())
	deserializedBlock := AltBlock{}
	assert.NoError(deserializedBlock.FromVbkEncoding(stream))

	assert.Equal(defaultAltBlock.Hash, deserializedBlock.Hash)
	assert.Equal(defaultAltBlock.Height, deserializedBlock.Height)
	assert.Equal(defaultAltBlock.Timestamp, deserializedBlock.Timestamp)
}
