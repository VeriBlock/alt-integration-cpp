package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/array.h>
// #include <string.h>
import "C"
import "unsafe"

func freeArrayU8(array *C.pop_array_u8_t) {
	C.pop_array_u8_free(array)
}

func createBytes(array *C.pop_array_u8_t) []byte {
	res := make([]byte, array.size)
	C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	return res
}
