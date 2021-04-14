package entities

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/array.h>
// #include <string.h>
import "C"
import "unsafe"

func ConvertToBytes(array *C.pop_array_u8_t) []byte {
	res := make([]byte, array.size)
	C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	return res
}
