// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/array.h>
// #include <string.h>
// #include <stdlib.h>
import "C"
import (
	"runtime"
	"unsafe"
)

func freeArrayU8(array *C.pop_array_u8_t) {
	C.pop_array_u8_free(array)
}

func createBytes(array *C.pop_array_u8_t) []byte {
	res := make([]byte, array.size)
	if array.size != 0 {
		C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	}
	return res
}

func createCBytes(bytes []byte) C.pop_array_u8_t {
	res := C.pop_array_u8_new(C.size_t(len(bytes)))
	if res.size != 0 {
		C.memcpy(unsafe.Pointer(res.data), unsafe.Pointer(&bytes[0]), res.size)
	}
	runtime.SetFinalizer(&res, func(v *C.pop_array_u8_t) {
		C.pop_array_u8_free(v)
	})
	return res
}

func freeCArrayDouble(array *C.pop_array_double_t) {
	C.pop_array_double_free(array)
}

func createCArrDouble(floats []float64) C.pop_array_double_t {
	res := C.pop_array_double_new(C.size_t(len(floats)))
	if res.size != 0 {
		C.memcpy(unsafe.Pointer(res.data), unsafe.Pointer(&floats[0]), res.size)
	}
	runtime.SetFinalizer(&res, func(v *C.pop_array_double_t) {
		C.pop_array_double_free(v)
	})
	return res
}

func createArrFloat64(array *C.pop_array_double_t) []float64 {
	res := make([]float64, array.size)
	if array.size != 0 {
		C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	}
	return res
}

func freeCArrayU32(array *C.pop_array_u32_t) {
	C.pop_array_u32_free(array)
}

func createCArrU32(nums []uint32) C.pop_array_u32_t {
	res := C.pop_array_u32_new(C.size_t(len(nums)))
	if res.size != 0 {
		C.memcpy(unsafe.Pointer(res.data), unsafe.Pointer(&nums[0]), res.size)
	}
	runtime.SetFinalizer(&res, func(v *C.pop_array_u32_t) {
		C.pop_array_u32_free(v)
	})
	return res
}

func createArrU32(array *C.pop_array_u32_t) []uint32 {
	res := make([]uint32, array.size)
	if array.size != 0 {
		C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	}
	return res
}

func freeArrayArrayU8(array *C.pop_array_array_u8_t) {
	C.pop_array_array_u8_free(array)
}

func createArrayOfArraysU8(array *C.pop_array_array_u8_t) [][]byte {
	res := make([][]byte, array.size)
	if array.size != 0 {
		for i := 0; i < len(res); i++ {
			arrU8 := C.pop_array_array_u8_at(array, C.size_t(i))
			arr := make([]byte, arrU8.size)
			C.memcpy(unsafe.Pointer(&arr[0]), unsafe.Pointer(arrU8.data), arrU8.size)
			res[i] = arr
		}
	}
	return res
}

func freeArrayChar(array *C.pop_array_string_t) {
	C.pop_array_string_free(array)
}

func createString(array *C.pop_array_string_t) string {
	res := make([]byte, array.size)
	if array.size != 0 {
		C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	}
	return string(res)
}

func createCString(str string) C.pop_array_string_t {
	res := C.pop_array_string_new(C.size_t(len(str)))
	if res.size != 0 {
		C.memcpy(unsafe.Pointer(res.data), unsafe.Pointer(&[]byte(str)[0]), res.size)
	}
	runtime.SetFinalizer(&res, func(v *C.pop_array_string_t) {
		C.pop_array_string_free(v)
	})
	return res
}
