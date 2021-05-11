// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import "testing"

func TestConfigFree(t *testing.T) {
	t.Parallel()

	config := NewConfig()
	config.Free()
	config.Free()
}

// TODO fix memory leaks
// func TestConfig(t *testing.T) {
//	t.Parallel()

// 	config := NewConfig()
// 	defer config.Free()

// 	t.Log("Run Config set function tests")
// 	config.SetStartOfSlope(1000)
// 	config.SetSlopeNormal(1000)
// 	config.SetSlopeKeystone(1000)
// 	config.SetSlopeKeystone(1000)
// 	config.SetKeystoneRound(1000)
// 	config.SetFlatScoreRound(1000)
// 	config.SetUseFlatScoreRound(false)
// 	config.SetMaxScoreThresholdNormal(1000)
// 	config.SetMaxScoreThresholdKeystone(1000)
// 	config.SetDifficultyAveragingInterval(1000)
// 	config.SetRoundRatios([]float64{0.5, 1.5})
// 	config.SetPopRewardsLookupTable([]float64{0.5, 1.5})
// 	config.SetMaxFutureBlockTime(1000)
// 	config.SetKeystoneInterval(1000)
// 	config.SetVbkFinalityDelay(1000)
// 	config.SetEndorsementSettlementInterval(1000)
// 	config.SetMaxPopDataSize(1000)
// 	config.SetForkResolutionLookupTable([]uint32{5, 15})

// 	if !config.SelectBtcParams("test", popbtcstartheight, &popbtcblocks) || !config.SelectBtcParams("regtest", 0, nil) {
// 		t.Error("Failed to select btc params")
// 	}

// 	if !config.SelectVbkParams("test", popvbkstartheight, &popvbkblocks) || !config.SelectVbkParams("regtest", 0, nil) {
// 		t.Error("Failed to select vbk params")
// 	}
// }
