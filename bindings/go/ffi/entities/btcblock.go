package entities

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/btcblock.h>
import "C"

type BtcBlock struct {
	ref *C.pop_btc_block_t
}

func (v *BtcBlock) GetHash() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return nil
}

func (v *BtcBlock) GetPreviousBlock() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return nil
}

func (v *BtcBlock) GetMerkleRoot() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return nil
}

func (v *BtcBlock) GetVersion() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_version(v.ref))
}

func (v *BtcBlock) GetTimestamp() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_timestamp(v.ref))
}

func (v *BtcBlock) GetNonce() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_nonce(v.ref))
}

func (v *BtcBlock) GetDifficulty() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_difficulty(v.ref))
}
