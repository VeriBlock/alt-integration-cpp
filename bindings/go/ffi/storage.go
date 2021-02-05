package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/c/storage.h>
import "C"
import "runtime"

type Storage struct {
	ref *C.Storage_t
}

func NewStorage(path string) *Storage {
	storage := &Storage{ref: C.VBK_NewStorage(C.CString(path))}
	runtime.SetFinalizer(storage, func(v *Storage) {
		v.Free()
	})
	return storage
}

func (v *Storage) Free() {
	if v.ref != nil {
		C.VBK_FreeStorage(v.ref)
		v.ref = nil
	}
}
