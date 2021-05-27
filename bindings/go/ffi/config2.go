// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/config2.h>
import "C"
import (
	"runtime"
)

// Config - Configuration for Veriblock integration
type Config2 struct {
	ref *C.pop_config_t
}

func (v *Config2) validate() {
	if v.ref == nil {
		panic("Config does not initialized")
	}
}

func NewConfig2() *Config2 {
	val := &Config2{ref: C.pop_config_new()}
	runtime.SetFinalizer(val, func(v *Config2) {
		v.Free()
	})
	return val
}

func (v *Config2) Free() {
	if v.ref != nil {
		C.pop_config_free(v.ref)
		v.ref = nil
	}
}

// SelectVbkParams - set VBK parameters and store them in config.
// Param "net" should be one of: "main", "test", "regtest", "alpha"
// Param "blocks" should be: "hex,hex,hex"
func (v *Config2) SelectVbkParams(net string, startHeight int, blocks string) {
	v.validate()
	C.pop_config_function_select_vbk_params(v.ref, createCString(net), C.int(startHeight), createCString(blocks))
}

// SelectBtcParams - set BTC parameters and store them in config.
// Param "net" should be one of: "main", "test", "regtest", "alpha"
// Param "blocks" should be: "hex,hex,hex"
func (v *Config2) SelectBtcParams(net string, startHeight int, blocks string) {
	v.validate()
	C.pop_config_function_select_btc_params(v.ref, createCString(net), C.int(startHeight), createCString(blocks))
}

// SetStartOfSlope ...
func (v *Config2) SetStartOfSlope(val float64) {
	v.validate()
	C.pop_config_set_start_of_slope(v.ref, C.double(val))
}

// SetSlopeNormal ...
func (v *Config2) SetSlopeNormal(val float64) {
	v.validate()
	C.pop_config_set_slope_normal(v.ref, C.double(val))
}

// SetSlopeKeystone ...
func (v *Config2) SetSlopeKeystone(val float64) {
	v.validate()
	C.pop_config_set_slope_keystone(v.ref, C.double(val))
}

// SetKeystoneRound ...
func (v *Config2) SetKeystoneRound(val uint32) {
	v.validate()
	C.pop_config_set_keystone_round(v.ref, C.uint32_t(val))
}

// SetFlatScoreRound ...
func (v *Config2) SetFlatScoreRound(val uint32) {
	v.validate()
	C.pop_config_set_flat_score_round(v.ref, C.uint32_t(val))
}

// SetUseFlatScoreRound ...
func (v *Config2) SetUseFlatScoreRound(val bool) {
	v.validate()
	C.pop_config_set_use_flat_score_round(v.ref, C.bool(val))
}

// SetMaxScoreThresholdNormal ...
func (v *Config2) SetMaxScoreThresholdNormal(val float64) {
	v.validate()
	C.pop_config_set_max_score_threshold_normal(v.ref, C.double(val))
}

// SetMaxScoreThresholdKeystone ...
func (v *Config2) SetMaxScoreThresholdKeystone(val float64) {
	v.validate()
	C.pop_config_set_max_score_threshold_keystone(v.ref, C.double(val))
}

// SetDifficultyAveragingInterval ...
func (v *Config2) SetDifficultyAveragingInterval(val uint32) {
	v.validate()
	C.pop_config_set_difficulty_averaging_interval(v.ref, C.uint32_t(val))
}

// SetRoundRatios ...
func (v *Config2) SetRoundRatios(vals []float64) {
	v.validate()
	C.pop_config_set_round_ratios(v.ref, createCArrDouble(vals))
}

// SetPopRewardsLookupTable ...
func (v *Config2) SetPopRewardsLookupTable(vals []float64) {
	v.validate()
	C.pop_config_set_pop_rewards_lookup_table(v.ref, createCArrDouble(vals))
}

// SetMaxFutureBlockTime ...
func (v *Config2) SetMaxFutureBlockTime(val uint32) {
	v.validate()
	C.pop_config_set_max_future_block_time(v.ref, C.uint32_t(val))
}

// SetKeystoneInterval ...
func (v *Config2) SetKeystoneInterval(val uint32) {
	v.validate()
	C.pop_config_set_keystone_interval(v.ref, C.uint32_t(val))
}

// SetVbkFinalityDelay ...
func (v *Config2) SetVbkFinalityDelay(val uint32) {
	v.validate()
	C.pop_config_set_vbk_finality_delay(v.ref, C.uint32_t(val))
}

// SetEndorsementSettlementInterval ...
func (v *Config2) SetEndorsementSettlementInterval(val uint32) {
	v.validate()
	C.pop_config_set_endorsement_settlement_interval(v.ref, C.uint32_t(val))
}

// SetMaxPopDataSize ...
func (v *Config2) SetMaxPopDataSize(val uint32) {
	v.validate()
	C.pop_config_set_max_pop_data_size(v.ref, C.uint32_t(val))
}

// SetForkResolutionLookupTable ...
func (v *Config2) SetForkResolutionLookupTable(vals []uint32) {
	v.validate()
	C.pop_config_set_fork_resolution_lookup_table(v.ref, createCArrU32(vals))
}

// SetPopPayoutDelay ...
func (v *Config2) SetPopPayoutDelay(val uint32) {
	v.validate()
	C.pop_config_set_pop_payout_delay(v.ref, C.uint32_t(val))
}

// GetStartOfSlope ...
func (v *Config2) GetStartOfSlope() float64 {
	v.validate()
	return float64(C.pop_config_get_start_of_slope(v.ref))
}

// GetSlopeNormal ...
func (v *Config2) GetSlopeNormal() float64 {
	v.validate()
	return float64(C.pop_config_get_slope_normal(v.ref))
}

// GetSlopeKeystone ...
func (v *Config2) GetSlopeKeystone() float64 {
	v.validate()
	return float64(C.pop_config_get_slope_keystone(v.ref))
}

// GetKeystoneRound ...
func (v *Config2) GetKeystoneRound() uint32 {
	v.validate()
	return uint32(C.pop_config_get_keystone_round(v.ref))
}

// GetFlatScoreRound ...
func (v *Config2) GetFlatScoreRound() uint32 {
	v.validate()
	return uint32(C.pop_config_get_flat_score_round(v.ref))
}

// GetUseFlatScoreRound ...
func (v *Config2) GetUseFlatScoreRound() bool {
	v.validate()
	return bool(C.pop_config_get_use_flat_score_round(v.ref))
}

// GetMaxScoreThresholdNormal ...
func (v *Config2) GetMaxScoreThresholdNormal() float64 {
	v.validate()
	return float64(C.pop_config_get_max_score_threshold_normal(v.ref))
}

// GetMaxScoreThresholdKeystone ...
func (v *Config2) GetMaxScoreThresholdKeystone() float64 {
	v.validate()
	return float64(C.pop_config_get_max_score_threshold_keystone(v.ref))
}

// GetDifficultyAveragingInterval ...
func (v *Config2) GetDifficultyAveragingInterval() uint32 {
	v.validate()
	return uint32(C.pop_config_get_difficulty_averaging_interval(v.ref))
}

// GetRoundRatios ...
func (v *Config2) GetRoundRatios() []float64 {
	v.validate()
	arr := C.pop_config_get_round_ratios(v.ref)
	defer freeCArrayDouble(&arr)
	return createArrFloat64(&arr)
}

// GetPopRewardsLookupTable ...
func (v *Config2) GetPopRewardsLookupTable() []float64 {
	v.validate()
	arr := C.pop_config_get_pop_rewards_lookup_table(v.ref)
	defer freeCArrayDouble(&arr)
	return createArrFloat64(&arr)
}

// GetMaxFutureBlockTime ...
func (v *Config2) GetMaxFutureBlockTime() uint32 {
	v.validate()
	return uint32(C.pop_config_get_max_future_block_time(v.ref))
}

// GetKeystoneInterval ...
func (v *Config2) GetKeystoneInterval() uint32 {
	v.validate()
	return uint32(C.pop_config_get_keystone_interval(v.ref))
}

// GetVbkFinalityDelay ...
func (v *Config2) GetVbkFinalityDelay() uint32 {
	v.validate()
	return uint32(C.pop_config_get_vbk_finality_delay(v.ref))
}

// GetEndorsementGettlementInterval ...
func (v *Config2) GetEndorsementSettlementInterval() uint32 {
	v.validate()
	return uint32(C.pop_config_get_endorsement_settlement_interval(v.ref))
}

// GetMaxPopDataSize ...
func (v *Config2) GetMaxPopDataSize() uint32 {
	v.validate()
	return uint32(C.pop_config_get_max_pop_data_size(v.ref))
}

// GetForkResolutionLookupTable ...
func (v *Config2) GetForkResolutionLookupTable() []uint32 {
	v.validate()
	arr := C.pop_config_get_fork_resolution_lookup_table(v.ref)
	defer freeCArrayU32(&arr)
	return createArrU32(&arr)
}

// GetPopPayoutDelay ...
func (v *Config2) GetPopPayoutDelay() uint32 {
	v.validate()
	return uint32(C.pop_config_get_pop_payout_delay(v.ref))
}

// GetMaxVbkBlocksInAltBlock ...
func (v *Config2) GetMaxVbkBlocksInAltBlock() uint32 {
	v.validate()
	return uint32(C.pop_config_get_max_vbk_blocks_in_alt_block(v.ref))
}

// GetMaxVTBsInAltBlock ...
func (v *Config2) GetMaxVTBsInAltBlock() uint32 {
	v.validate()
	return uint32(C.pop_config_get_max_vtbs_in_alt_block(v.ref))
}

// GetMaxATVsInAltBlock ...
func (v *Config2) GetMaxATVsInAltBlock() uint32 {
	v.validate()
	return uint32(C.pop_config_get_max_atvs_in_alt_block(v.ref))
}

// GetFinalityDelay ...
func (v *Config2) GetFinalityDelay() uint32 {
	v.validate()
	return uint32(C.pop_config_get_finality_delay(v.ref))
}

// GetMaxAltchainFutureBlockTime ...
func (v *Config2) GetMaxAltchainFutureBlockTime() uint32 {
	v.validate()
	return uint32(C.pop_config_get_max_altchain_future_block_time(v.ref))
}

// AltGetBootstrapBlock ...
func (v *Config2) AltGetBootstrapBlock() *AltBlock {
	v.validate()
	return createAltBlock(C.pop_config_get_alt_bootstrap_block(v.ref))
}

// GetVbkNetworkName ...
func (v *Config2) GetVbkNetworkName() string {
	v.validate()
	c_str := C.pop_config_get_vbk_network_name(v.ref)
	defer freeArrayChar(&c_str)
	return createString(&c_str)
}

// GetBtcNetworkName ...
func (v *Config2) GetBtcNetworkName() string {
	v.validate()
	c_str := C.pop_config_get_btc_network_name(v.ref)
	defer freeArrayChar(&c_str)
	return createString(&c_str)
}
