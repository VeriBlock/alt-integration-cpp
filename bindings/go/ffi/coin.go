// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/coin.h>
import "C"
import "runtime"

type Coin struct {
	ref *C.pop_coin_t
}

func (v *Coin) validate() {
	if v.ref == nil {
		panic("Coin does not initialized")
	}
}

func GenerateDefaultCoin() *Coin {
	val := &Coin{ref: C.pop_coin_generate_default_value()}
	runtime.SetFinalizer(val, func(v *Coin) {
		v.Free()
	})
	return val
}

func (v *Coin) Free() {
	if v.ref != nil {
		C.pop_coin_free(v.ref)
		v.ref = nil
	}
}

func (v *Coin) GetUnits() int64 {
	v.validate()
	return int64(C.pop_coin_get_units(v.ref))
}
