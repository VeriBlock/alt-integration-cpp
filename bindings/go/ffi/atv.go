// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb  -lm
// #include <veriblock/pop/c/entities/atv.h>
import "C"
import (
	"runtime"

	"github.com/stretchr/testify/assert"
)

type Atv struct {
	ref *C.pop_atv_t
}

func GenerateDefaultAtv() *Atv {
	val := &Atv{ref: C.pop_atv_generate_default_value()}
	runtime.SetFinalizer(val, func(v *Atv) {
		v.Free()
	})
	return val
}

func createAtv(ref *C.pop_atv_t) *Atv {
	val := &Atv{ref: ref}
	runtime.SetFinalizer(val, func(v *Atv) {
		v.Free()
	})
	return val
}

func freeArrayAtv(array *C.pop_array_atv_t) {
	C.pop_array_atv_free(array)
}

func createArrayAtv(array *C.pop_array_atv_t) []*Atv {
	res := make([]*Atv, array.size, array.size)
	for i := 0; i < len(res); i++ {
		res[i] = createAtv(C.pop_array_atv_at(array, C.size_t(i)))
	}
	return res
}

func (v *Atv) Free() {
	if v.ref != nil {
		C.pop_atv_free(v.ref)
		v.ref = nil
	}
}

func (v *Atv) GetBlockOfProof() *VbkBlock {
	if v.ref == nil {
		panic("Vtb does not initialized")
	}
	return createVbkBlock(C.pop_atv_get_block_of_proof(v.ref))
}

func (val1 *Atv) assertEquals(assert *assert.Assertions, val2 *Atv) {
	val1.GetBlockOfProof().assertEquals(assert, val2.GetBlockOfProof())
}
