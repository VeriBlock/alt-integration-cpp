// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/pop_payouts.h>
import "C"
import "runtime"

type PopPayout struct {
	ref *C.pop_pop_payout_t
}

func generateDefaultPopPayout() *PopPayout {
	return createPopPayout(C.pop_pop_payout_generate_default_value())
}

func createPopPayout(ref *C.pop_pop_payout_t) *PopPayout {
	val := &PopPayout{ref: ref}
	runtime.SetFinalizer(val, func(v *PopPayout) {
		v.Free()
	})
	return val
}

func (v *PopPayout) Free() {
	if v.ref != nil {
		C.pop_pop_payout_free(v.ref)
		v.ref = nil
	}
}

func (v *PopPayout) GetPayoutInfo() []byte {
	if v.ref == nil {
		panic("PopPayout does not initialized")
	}
	array := C.pop_pop_payout_get_payout_info(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *PopPayout) GetAmount() uint64 {
	if v.ref == nil {
		panic("PopPayout does not initialized")
	}
	return uint64(C.pop_pop_payout_get_amount(v.ref))
}
