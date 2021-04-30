// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/config2.h>
import "C"
import "runtime"

// Config - Configuration for Veriblock integration
type Config2 struct {
	ref *C.pop_config_t
}

func NewConfig2() *Config2 {
	val := &Config2{ref: C.pop_config_new()}
	runtime.SetFinalizer(val, func(v *Config2) {
		v.Free()
	})
	return val
}

// SelectVbkParams - set VBK parameters and store them in config.
// Param "net" should be one of: "main", "test", "regtest", "alpha"
// Param "blocks" should be: "hex,hex,hex"
func (v *Config2) SelectVbkParams(net string, startHeight int, blocks string) {
	C.pop_config_function_select_vbk_params(v.ref, createCString(net), C.int(startHeight), createCString(blocks))
}

// SelectBtcParams - set BTC parameters and store them in config.
// Param "net" should be one of: "main", "test", "regtest", "alpha"
// Param "blocks" should be: "hex,hex,hex"
func (v *Config2) SelectBtcParams(net string, startHeight int, blocks string) {
	C.pop_config_function_select_btc_params(v.ref, createCString(net), C.int(startHeight), createCString(blocks))
}

func (v *Config2) Free() {
	if v.ref != nil {
		C.pop_config_free(v.ref)
		v.ref = nil
	}
}
