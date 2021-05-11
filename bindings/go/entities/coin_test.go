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

func TestCoinRound(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	coin := Coin{1}
	stream := new(bytes.Buffer)
	err := coin.ToVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal([]byte{1, 1}, stream.Bytes())
	decoded := Coin{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(coin.Units, decoded.Units)
	assert.Equal(0, stream.Len(), "Stream has more data")
}
