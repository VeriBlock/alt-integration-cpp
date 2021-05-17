// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestConfig2Free(t *testing.T) {
	t.Parallel()

	config := NewConfig2()

	config.Free()
	config.Free()
}

func TestConfig2(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	config := NewConfig2()
	defer config.Free()

	config.SetStartOfSlope(1000)
	assert.Equal(float64(1000), config.GetStartOfSlope())

	config.SetSlopeNormal(1000)
	assert.Equal(float64(1000), config.GetSlopeNormal())

	config.SetSlopeKeystone(1000)
	assert.Equal(float64(1000), config.GetSlopeKeystone())

	config.SetKeystoneRound(1000)
	assert.Equal(uint32(1000), config.GetKeystoneRound())

	config.SetFlatScoreRound(1000)
	assert.Equal(uint32(1000), config.GetFlatScoreRound())

	config.SetUseFlatScoreRound(true)
	assert.Equal(true, config.GetUseFlatScoreRound())

	config.SetMaxScoreThresholdNormal(1000)
	assert.Equal(float64(1000), config.GetMaxScoreThresholdNormal())

	config.SetMaxScoreThresholdKeystone(1000)
	assert.Equal(float64(1000), config.GetMaxScoreThresholdKeystone())

	config.SetDifficultyAveragingInterval(1000)
	assert.Equal(uint32(1000), config.GetDifficultyAveragingInterval())

	// config.SetRoundRatios([]float64{0.5, 1.5})
	// assert.Equal([]float64{0.5, 1.5}, config.GetRoundRatios())

	// config.SetPopRewardsLookupTable([]float64{0.5, 1.5})
	// assert.Equal([]float64{0.5, 1.5}, config.GetPopRewardsLookupTable())

	config.SetMaxFutureBlockTime(1000)
	assert.Equal(uint32(1000), config.GetMaxFutureBlockTime())

	config.SetKeystoneInterval(1000)
	assert.Equal(uint32(1000), config.GetKeystoneInterval())

	config.SetVbkFinalityDelay(1000)
	assert.Equal(uint32(1000), config.GetVbkFinalityDelay())

	config.SetEndorsementSettlementInterval(1000)
	assert.Equal(uint32(1000), config.GetEndorsementSettlementInterval())

	config.SetMaxPopDataSize(1000)
	assert.Equal(uint32(1000), config.GetMaxPopDataSize())

	// config.SetForkResolutionLookupTable([]uint32{5, 15})
	// assert.Equal([]uint32{5, 15}, config.GetForkResolutionLookupTable())

	config.SetPopPayoutDelay(1000)
	assert.Equal(uint32(1000), config.GetPopPayoutDelay())
}
