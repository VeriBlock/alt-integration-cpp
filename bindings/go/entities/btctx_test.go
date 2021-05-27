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

func TestBtcTxRound(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	btcTx := BtcTx{[]byte{1, 2, 3}}
	stream := new(bytes.Buffer)
	assert.NoError(btcTx.ToVbkEncoding(stream))
	decoded := BtcTx{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(btcTx.Tx, decoded.Tx)
	assert.Equal(0, stream.Len(), "Stream has more data")
}
