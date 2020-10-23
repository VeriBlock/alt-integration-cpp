package ffi

import "testing"

func TestConfig(t *testing.T) {
	config := NewConfig()
	defer config.Free()

	t.Log("Run Config set function tests")
	config.SetStartOfSlope(1000)
	config.SetSlopeNormal(1000)
	config.SetSlopeKeystone(1000)
	config.SetSlopeKeystone(1000)
	config.SetKeystoneRound(1000)
	config.SetFlatScoreRound(1000)
	config.SetUseFlatScoreRound(false)
	config.SetMaxScoreThresholdNormal(1000)
	config.SetMaxScoreThresholdKeystone(1000)
	config.SetDifficultyAveragingInterval(1000)
	config.SetRoundRatios([]float64{0.5, 1.5})
	config.SetPopRewardsLookupTable([]float64{0.5, 1.5})
	config.SetMaxFutureBlockTime(1000)
	config.SetKeystoneInterval(1000)
	config.SetVbkFinalityDelay(1000)
	config.SetEndorsementSettlementInterval(1000)
	config.SetMaxPopDataSize(1000)
	config.SetForkResolutionLookupTable([]uint32{5, 15})

	// !config.SelectVbkParams("regtest", popvbkstartheight, &popvbkblocks) ||
	if !config.SelectVbkParams("regtest", popvbkstartheight, nil) {
		t.Error("Failed to select vbk params")
	}
	// !config.SelectBtcParams("regtest", popbtcstartheight, &popbtcblocks) ||
	if !config.SelectBtcParams("regtest", popbtcstartheight, nil) {
		t.Error("Failed to select btc params")
	}
}
