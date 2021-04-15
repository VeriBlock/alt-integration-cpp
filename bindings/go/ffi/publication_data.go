package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/publication_data.h>
import "C"

type PublicationData struct {
	ref *C.pop_publication_data_t
}

func (v *PublicationData) Free() {
	if v.ref != nil {
		C.pop_publication_data_free(v.ref)
		v.ref = nil
	}
}