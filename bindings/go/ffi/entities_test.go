// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestBtcBlock(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	btc_block := generateDefaultBtcBlock()

	assert.Equal(btc_block.GetDifficulty(), uint32(1))
	assert.Equal(btc_block.GetNonce(), uint32(1))
	assert.Equal(btc_block.GetTimestamp(), uint32(1))
	assert.Equal(btc_block.GetVersion(), uint32(1))
	assert.Equal(btc_block.GetMerkleRoot(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(btc_block.GetPreviousBlock(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(btc_block.GetHash(), []byte{0x16, 0x4a, 0x73, 0xce, 0x49, 0xbb, 0x95, 0xdf, 0x8f, 0x54, 0x71, 0x69, 0xd5, 0x73, 0x1f, 0x06,
		0x9a, 0x83, 0x71, 0x4b, 0xb4, 0x31, 0xaf, 0xa0, 0x9a, 0x24, 0xe0, 0x88, 0xc9, 0xff, 0x05, 0x1c})

	btc_block.Free()
	btc_block.Free()
}

func TestVbkBlock(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	vbk_block := generateDefaultVbkBlock()

	assert.Equal(vbk_block.GetHeight(), int32(1))
	assert.Equal(vbk_block.GetNonce(), uint64(1))
	assert.Equal(vbk_block.GetDifficulty(), int32(1))
	assert.Equal(vbk_block.GetVersion(), int16(1))
	assert.Equal(vbk_block.GetTimestamp(), uint32(1))
	assert.Equal(vbk_block.GetMerkleRoot(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(vbk_block.GetPreviousBlock(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(vbk_block.GetPreviousKeystone(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(vbk_block.GetSecondPreviousKeystone(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(vbk_block.GetHash(), []byte{0xc8, 0x43, 0x1c, 0x39, 0x10, 0x4b, 0x66, 0x9d, 0x23,
		0x4f, 0x54, 0xb2, 0xa6, 0xb7, 0x6a, 0xdb, 0x58, 0x1c, 0x53, 0x52, 0xca, 0xd8, 0x18, 0x26})
	assert.Equal(vbk_block.GetID(), []byte{0xa6, 0xb7, 0x6a, 0xdb, 0x58, 0x1c, 0x53, 0x52, 0xca, 0xd8, 0x18, 0x26})

	vbk_block.Free()
	vbk_block.Free()
}

func TestAltBlock(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	alt_block := generateDefaultAltBlock()

	assert.Equal(alt_block.GetHeight(), int32(1))
	assert.Equal(alt_block.GetTimestamp(), uint32(1))
	assert.Equal(alt_block.GetHash(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(alt_block.GetPreviousBlock(), []byte{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2})

	alt_block.Free()
	alt_block.Free()
}

func TestVtb(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	vtb := generateDefaultVtb()
	vbk_block := generateDefaultVbkBlock()

	vtb.GetContainingBlock().assertEquals(assert, vbk_block)

	assert.Equal(vtb.GetID(), []byte{
		0xdc, 0x3d, 0xde, 0x98, 0xde,
		0x60, 0xfc, 0xcb, 0x20, 0xd1,
		0x45, 0x18, 0x4c, 0x8d, 0x46,
		0x3b, 0x1b, 0x52, 0x36, 0x58,
		0x8a, 0xf7, 0xa8, 0x01, 0x44,
		0x72, 0x7b, 0xf1, 0xf7, 0x87,
		0xf2, 0xcb,
	})

	vtb.Free()
	vtb.Free()
}

func TestAtv(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	atv := generateDefaultAtv()
	vbk_block := generateDefaultVbkBlock()

	atv.GetBlockOfProof().assertEquals(assert, vbk_block)

	assert.Equal(atv.GetID(), []byte{
		0x2b, 0x30, 0x75, 0x20, 0xcc,
		0xd0, 0x2d, 0x78, 0x17, 0x61,
		0x97, 0xdf, 0x05, 0x4c, 0x20,
		0x71, 0xca, 0xf0, 0x5f, 0xac,
		0xeb, 0x6a, 0xa6, 0x18, 0xa9,
		0xb4, 0x55, 0xc8, 0x25, 0x06,
		0xa0, 0x06,
	})

	atv.Free()
	atv.Free()
}

func TestPopData(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	pop_data := generateDefaultPopData()
	atv := generateDefaultAtv()
	vtb := generateDefaultVtb()
	vbk_block := generateDefaultVbkBlock()

	atvs := pop_data.GetAtvs()
	vtbs := pop_data.GetVtbs()
	context := pop_data.GetContext()

	assert.Equal(len(atvs), 10)
	assert.Equal(len(vtbs), 10)
	assert.Equal(len(context), 10)
	for _, v := range atvs {
		v.assertEquals(assert, atv)
	}
	for _, v := range vtbs {
		v.assertEquals(assert, vtb)
	}
	for _, v := range context {
		v.assertEquals(assert, vbk_block)
	}

	pop_data.Free()
	pop_data.Free()
}

func TestNetworkBytePair(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	nbp := generateDefaultNetworkBytePair()

	assert.Equal(nbp.HasNetworkByte(), true)
	assert.Equal(nbp.GetNetworkByte(), uint8(1))
	assert.Equal(nbp.GetTypeID(), uint8(1))

	nbp.Free()
	nbp.Free()
}

func TestAddress(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	addr := generateDefaultAddress()

	assert.Equal(addr.GetAddressType(), uint8(1))
	assert.Equal(addr.GetAddress(), "V111111111111111111111111G3LuZ")

	addr.Free()
	addr.Free()
}

func TestBtcTx(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	btctx := generateDefaultBtcTx()

	assert.Equal(btctx.GetTx(), []byte{1, 1, 1, 1, 1})

	btctx.Free()
	btctx.Free()
}

func TestCoin(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	coin := generateDefaultCoin()

	assert.Equal(coin.GetUnits(), int64(1))

	coin.Free()
	coin.Free()
}

func TestMerklePath(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	mp := generateDefaultMerklePath()
	sub := mp.GetSubject()
	layers := mp.GetLayers()
	bytes := make([]byte, 32)
	bytes[31] = 1

	assert.Equal(mp.GetIndex(), int32(1))
	assert.Equal(sub, bytes)
	assert.Equal(len(layers), 3)
	for _, layer := range layers {
		assert.Equal(layer, bytes)
	}

	mp.Free()
	mp.Free()
}

func TestPopPayout(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	popPayout := generateDefaultPopPayout()

	assert.Equal(popPayout.GetPayoutInfo(), []byte{1, 2, 3, 4})
	assert.Equal(popPayout.GetAmount(), uint64(300))

	popPayout.Free()
	popPayout.Free()
}
