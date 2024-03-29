// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestConfigFree(t *testing.T) {

	config := NewConfig()

	config.Free()
	config.Free()
}

func TestConfig(t *testing.T) {

	assert := assert.New(t)

	config := NewConfig()
	defer config.Free()

	SetOnGetBootstrapBlock(func() AltBlock {
		return *GenerateDefaultAltBlock()
	})

	config.SelectVbkParams("regtest", 0, "")
	config.SelectBtcParams("regtest", 0, "")

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

	config.SetRoundRatios([]float64{0.5, 1.5})
	assert.Equal([]float64{0.5, 1.5}, config.GetRoundRatios())

	config.SetPopRewardsLookupTable([]float64{0.5, 1.5})
	assert.Equal([]float64{0.5, 1.5}, config.GetPopRewardsLookupTable())

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

	config.SetForkResolutionLookupTable([]uint32{5, 15})
	assert.Equal([]uint32{5, 15}, config.GetForkResolutionLookupTable())

	config.SetPopPayoutDelay(1000)
	assert.Equal(uint32(1000), config.GetPopPayoutDelay())

	config.SetMaxReorgDistance(1000)
	assert.Equal(uint32(1000), config.GetMaxReorgDistance())

	res := config.GetMaxVbkBlocksInAltBlock()
	assert.NotEqual(uint32(0), res)

	res = config.GetMaxVTBsInAltBlock()
	assert.NotEqual(uint32(0), res)

	res = config.GetMaxATVsInAltBlock()
	assert.NotEqual(uint32(0), res)

	res = config.GetFinalityDelay()
	assert.NotEqual(uint32(0), res)

	res = config.GetMaxAltchainFutureBlockTime()
	assert.NotEqual(uint32(0), res)

	alt := config.AltGetBootstrapBlock()
	assert.NotEqual(nil, alt)

	name := config.GetVbkNetworkName()
	assert.NotEqual("", name)

	name = config.GetBtcNetworkName()
	assert.NotEqual("", name)
}
