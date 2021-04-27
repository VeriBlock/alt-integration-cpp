// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lm
// #include <veriblock/pop/c/entities/btctx.h>
import "C"
import "runtime"

type BtcTx struct {
	ref *C.pop_btctx_t
}

func GenerateDefaultBtcTx() *BtcTx {
	val := &BtcTx{ref: C.pop_btctx_generate_default_value()}
	runtime.SetFinalizer(val, func(v *BtcTx) {
		v.Free()
	})
	return val
}

func (v *BtcTx) Free() {
	if v.ref != nil {
		C.pop_btctx_free(v.ref)
		v.ref = nil
	}
}

func (v *BtcTx) GetTx() []byte {
	if v.ref == nil {
		panic("BtcTx does not initialized")
	}
	array := C.pop_btctx_get_tx(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}
