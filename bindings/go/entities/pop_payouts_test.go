// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultPopPayoutsEncoded = "010309616464726573735f31000000000000000a09616464726573735f3200000000000000d509616464726573735f33000000000000013a"

	defaultPopPayouts = PopPayouts{[]Payout{
		{[]byte("address_1"), 10},
		{[]byte("address_2"), 213},
		{[]byte("address_3"), 314},
	}}
)

func TestPopPayoutsDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := bytes.NewReader(parseHex(defaultPopPayoutsEncoded))
	decoded := PopPayouts{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(len(defaultPopPayouts.payouts), 3)
	assert.Equal(len(decoded.payouts), len(defaultPopPayouts.payouts))
	// 0
	assert.Equal(decoded.payouts[0].Amount, defaultPopPayouts.payouts[0].Amount)
	assert.True(bytes.Equal(decoded.payouts[0].PayoutInfo, defaultPopPayouts.payouts[0].PayoutInfo))
	// 1
	assert.Equal(decoded.payouts[1].Amount, defaultPopPayouts.payouts[1].Amount)
	assert.True(bytes.Equal(decoded.payouts[1].PayoutInfo, defaultPopPayouts.payouts[1].PayoutInfo))
	// 2
	assert.Equal(decoded.payouts[2].Amount, defaultPopPayouts.payouts[2].Amount)
	assert.True(bytes.Equal(decoded.payouts[2].PayoutInfo, defaultPopPayouts.payouts[2].PayoutInfo))

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestPopPayoutsSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultPopPayouts.ToVbkEncoding(stream))
	encoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultPopPayoutsEncoded, encoded)
}

func TestPopPayoutsRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := bytes.NewReader(parseHex(defaultPopPayoutsEncoded))
	decoded := PopPayouts{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(len(defaultPopPayouts.payouts), 3)
	assert.Equal(len(decoded.payouts), len(defaultPopPayouts.payouts))
	// 0
	assert.Equal(decoded.payouts[0].Amount, defaultPopPayouts.payouts[0].Amount)
	assert.True(bytes.Equal(decoded.payouts[0].PayoutInfo, defaultPopPayouts.payouts[0].PayoutInfo))
	// 1
	assert.Equal(decoded.payouts[1].Amount, defaultPopPayouts.payouts[1].Amount)
	assert.True(bytes.Equal(decoded.payouts[1].PayoutInfo, defaultPopPayouts.payouts[1].PayoutInfo))
	// 2
	assert.Equal(decoded.payouts[2].Amount, defaultPopPayouts.payouts[2].Amount)
	assert.True(bytes.Equal(decoded.payouts[2].PayoutInfo, defaultPopPayouts.payouts[2].PayoutInfo))

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	reEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultPopPayoutsEncoded, reEncoded)
}
