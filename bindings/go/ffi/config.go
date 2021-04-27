// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/config.h>
import "C"
import (
	"runtime"
	"unsafe"
)

// Config - Configuration for Veriblock integration
type Config struct {
	ref *C.Config_t
}

// NewConfig - Returns instance of Config struct. Allocates memory.
func NewConfig() *Config {
	config := &Config{ref: C.VBK_NewConfig()}
	runtime.SetFinalizer(config, func(v *Config) {
		v.Free()
	})
	return config
}

// Free - Dealocates memory allocated for the config.
func (v *Config) Free() {
	if v.ref != nil {
		C.VBK_FreeConfig(v.ref)
		v.ref = nil
	}
}

// SelectVbkParams - set VBK parameters and store them in config.
// Param "net" should be one of: "main", "test", "regtest", "alpha"
// Param "blocks" should be: "hex,hex,hex"
func (v *Config) SelectVbkParams(net string, startHeight int, blocks *string) bool {
	var blocksArg *C.char
	if blocks == nil {
		blocksArg = nil
	} else {
		blocksArg = C.CString(*blocks)
	}
	res := C.VBK_SelectVbkParams(v.ref, C.CString(net), C.int(startHeight), blocksArg)
	return bool(res)
}

// SelectBtcParams - set BTC parameters and store them in config.
// Param "net" should be one of: "main", "test", "regtest", "alpha"
// Param "blocks" should be: "hex,hex,hex"
func (v *Config) SelectBtcParams(net string, startHeight int, blocks *string) bool {
	var blocksArg *C.char
	if blocks == nil {
		blocksArg = nil
	} else {
		blocksArg = C.CString(*blocks)
	}
	res := C.VBK_SelectBtcParams(v.ref, C.CString(net), C.int(startHeight), blocksArg)
	return bool(res)
}

// SetStartOfSlope ...
func (v *Config) SetStartOfSlope(val float64) {
	C.VBK_SetStartOfSlope(v.ref, C.double(val))
}

// SetSlopeNormal ...
func (v *Config) SetSlopeNormal(val float64) {
	C.VBK_SetSlopeNormal(v.ref, C.double(val))
}

// SetSlopeKeystone ...
func (v *Config) SetSlopeKeystone(val float64) {
	C.VBK_SetSlopeKeystone(v.ref, C.double(val))
}

// SetKeystoneRound ...
func (v *Config) SetKeystoneRound(val uint32) {
	C.VBK_SetKeystoneRound(v.ref, C.uint32_t(val))
}

// SetFlatScoreRound ...
func (v *Config) SetFlatScoreRound(val uint32) {
	C.VBK_SetFlatScoreRound(v.ref, C.uint32_t(val))
}

// SetUseFlatScoreRound ...
func (v *Config) SetUseFlatScoreRound(val bool) {
	C.VBK_SetUseFlatScoreRound(v.ref, C.bool(val))
}

// SetMaxScoreThresholdNormal ...
func (v *Config) SetMaxScoreThresholdNormal(val float64) {
	C.VBK_SetMaxScoreThresholdNormal(v.ref, C.double(val))
}

// SetMaxScoreThresholdKeystone ...
func (v *Config) SetMaxScoreThresholdKeystone(val float64) {
	C.VBK_SetMaxScoreThresholdKeystone(v.ref, C.double(val))
}

// SetDifficultyAveragingInterval ...
func (v *Config) SetDifficultyAveragingInterval(val uint32) {
	C.VBK_SetDifficultyAveragingInterval(v.ref, C.uint32_t(val))
}

// SetRoundRatios ...
func (v *Config) SetRoundRatios(vals []float64) {
	valsC := (*C.double)(unsafe.Pointer(&vals[0]))
	C.VBK_SetRoundRatios(v.ref, valsC, C.int(len(vals)))
}

// SetPopRewardsLookupTable ...
func (v *Config) SetPopRewardsLookupTable(vals []float64) {
	valsC := (*C.double)(unsafe.Pointer(&vals[0]))
	C.VBK_SetPopRewardsLookupTable(v.ref, valsC, C.int(len(vals)))
}

// SetMaxFutureBlockTime ...
func (v *Config) SetMaxFutureBlockTime(val uint32) {
	C.VBK_SetMaxFutureBlockTime(v.ref, C.uint32_t(val))
}

// SetKeystoneInterval ...
func (v *Config) SetKeystoneInterval(val uint32) {
	C.VBK_SetKeystoneInterval(v.ref, C.uint32_t(val))
}

// SetVbkFinalityDelay ...
func (v *Config) SetVbkFinalityDelay(val uint32) {
	C.VBK_SetVbkFinalityDelay(v.ref, C.uint32_t(val))
}

// SetEndorsementSettlementInterval ...
func (v *Config) SetEndorsementSettlementInterval(val uint32) {
	C.VBK_SetEndorsementSettlementInterval(v.ref, C.uint32_t(val))
}

// SetMaxPopDataSize ...
func (v *Config) SetMaxPopDataSize(val uint32) {
	C.VBK_SetMaxPopDataSize(v.ref, C.uint32_t(val))
}

// SetForkResolutionLookupTable ...
func (v *Config) SetForkResolutionLookupTable(vals []uint32) {
	valsC := (*C.uint32_t)(unsafe.Pointer(&vals[0]))
	C.VBK_SetForkResolutionLookupTable(v.ref, valsC, C.int(len(vals)))
}

// SetPopPayoutDelay ...
func (v *Config) SetPopPayoutDelay(val int32) {
	C.VBK_SetPopPayoutDelay(v.ref, C.int32_t(val))
}

// GetMaxPopDataSize ...
func (v *Config) GetMaxPopDataSize() uint32 {
	res := C.VBK_GetMaxPopDataSize(v.ref)
	return uint32(res)
}

// GetMaxVbkBlocksInAltBlock ...
func (v *Config) GetMaxVbkBlocksInAltBlock() int {
	res := C.VBK_GetMaxVbkBlocksInAltBlock(v.ref)
	return int(res)
}

// GetMaxVTBsInAltBlock ...
func (v *Config) GetMaxVTBsInAltBlock() int {
	res := C.VBK_GetMaxVTBsInAltBlock(v.ref)
	return int(res)
}

// GetMaxATVsInAltBlock ...
func (v *Config) GetMaxATVsInAltBlock() int {
	res := C.VBK_GetMaxATVsInAltBlock(v.ref)
	return int(res)
}

// GetEndorsementSettlementInterval ...
func (v *Config) GetEndorsementSettlementInterval() int32 {
	res := C.VBK_GetEndorsementSettlementInterval(v.ref)
	return int32(res)
}

// GetFinalityDelay ...
func (v *Config) GetFinalityDelay() uint32 {
	res := C.VBK_GetFinalityDelay(v.ref)
	return uint32(res)
}

// GetKeystoneInterval ...
func (v *Config) GetKeystoneInterval() uint32 {
	res := C.VBK_GetKeystoneInterval(v.ref)
	return uint32(res)
}

// GetMaxAltchainFutureBlockTime ...
func (v *Config) GetMaxAltchainFutureBlockTime() uint32 {
	res := C.VBK_GetMaxAltchainFutureBlockTime(v.ref)
	return uint32(res)
}

// AltGetBootstrapBlock ...
func (v *Config) AltGetBootstrapBlock() *VbkByteStream {
	res := C.VBK_AltGetBootstrapBlock(v.ref)
	return NewVbkByteStream(res)
}

// GetVbkNetworkName ...
func (v *Config) GetVbkNetworkName() string {
	c_str := C.VBK_GetVbkNetworkName(v.ref)
	return C.GoString(c_str)
}

// GetBtcNetworkName ...
func (v *Config) GetBtcNetworkName() string {
	c_str := C.VBK_GetBtcNetworkName(v.ref)
	return C.GoString(c_str)
}
