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

func (v *PopPayout) validate() {
	if v.ref == nil {
		panic("PopPayout does not initialized")
	}
}

func GenerateDefaultPopPayout() *PopPayout {
	return createPopPayout(C.pop_pop_payout_generate_default_value())
}

func createPopPayout(ref *C.pop_pop_payout_t) *PopPayout {
	val := &PopPayout{ref: ref}
	runtime.SetFinalizer(val, func(v *PopPayout) {
		v.Free()
	})
	return val
}

func freeArrayPopPayout(array *C.pop_array_pop_payout_t) {
	C.pop_array_pop_payout_free(array)
}

func createArrayPopPayout(array *C.pop_array_pop_payout_t) []*PopPayout {
	res := make([]*PopPayout, array.size, array.size)
	for i := 0; i < len(res); i++ {
		res[i] = createPopPayout(C.pop_array_pop_payout_at(array, C.size_t(i)))
	}
	return res
}

func (v *PopPayout) Free() {
	if v.ref != nil {
		C.pop_pop_payout_free(v.ref)
		v.ref = nil
	}
}

func (v *PopPayout) GetPayoutInfo() []byte {
	v.validate()
	array := C.pop_pop_payout_get_payout_info(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *PopPayout) GetAmount() uint64 {
	v.validate()
	return uint64(C.pop_pop_payout_get_amount(v.ref))
}
