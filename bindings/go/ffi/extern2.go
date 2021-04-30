// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/extern2.h>
// #include <string.h>
import "C"
import "unsafe"

// Exported functions
var (
	onGetAltchainID      = func() int64 { panic("onGetAltchainID not set") }
	onGetBootstrapBlock  = func() AltBlock { panic("onGetBootstrapBlock not set") }
	onGetBlockHeaderHash = func(toBeHashed []byte) []byte { panic("onGetBlockHeaderHash not set") }
	onCheckBlockHeader   = func(header []byte, root []byte) bool { panic("onCheckBlockHeader not set") }
)

//export pop_extern_function_get_altchain_id
func pop_extern_function_get_altchain_id() C.int64_t {
	return C.int64_t(onGetAltchainID())
}

//export pop_extern_function_get_bootstrap_block
func pop_extern_function_get_bootstrap_block() *C.pop_alt_block_t {
	return onGetBootstrapBlock().ref
}

//export pop_extern_function_get_block_header_hash
func pop_extern_function_get_block_header_hash(bytes C.pop_array_u8_t) C.pop_array_u8_t {
	hash := onGetBlockHeaderHash(createBytes(&bytes))

	var res C.pop_array_u8_t
	res.size = C.size_t(len(hash))
	C.memcpy(unsafe.Pointer(res.data), unsafe.Pointer(&hash[0]), res.size)

	return res
}

//export pop_extern_function_check_block_header
func pop_extern_function_check_block_header(header C.pop_array_u8_t, root C.pop_array_u8_t) C.bool {
	return C.bool(onCheckBlockHeader(createBytes(&header), createBytes(&root)))
}
