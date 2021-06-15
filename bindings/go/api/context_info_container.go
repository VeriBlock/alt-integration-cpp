// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/context_info_container.h>
import "C"
import "runtime"

type ContextInfoContainer struct {
	ref *C.pop_context_info_container_t
}

func (v *ContextInfoContainer) validate() {
	if v.ref == nil {
		panic("ContextInfoContainer does not initialized")
	}
}

func GenerateDefaultContextInfoContainer() *ContextInfoContainer {
	return createContextInfoContainer(C.pop_context_info_container_generate_default_value())
}

func createContextInfoContainer(ref *C.pop_context_info_container_t) *ContextInfoContainer {
	val := &ContextInfoContainer{ref: ref}
	runtime.SetFinalizer(val, func(v *ContextInfoContainer) {
		v.Free()
	})
	return val
}

func (v *ContextInfoContainer) Free() {
	if v.ref != nil {
		C.pop_context_info_container_free(v.ref)
		v.ref = nil
	}
}

func (v *ContextInfoContainer) GetFirstPreviousKeystone() []byte {
	v.validate()
	array := C.pop_context_info_container_get_first_previous_keystone(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *ContextInfoContainer) GetSecondPreviousKeystone() []byte {
	v.validate()
	array := C.pop_context_info_container_get_second_previous_keystone(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}
