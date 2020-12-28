package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/config.h>
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
	C.VBK_SetPopPayoutDelay(v.ref, C.int32(val))
}
