// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/stretchr/testify/assert"
)

var (
	networkByte = veriblock.NetworkBytePair{
		HasNetworkByte: false,
		NetworkByte:    0,
		TypeID:         uint8(veriblock.TxTypeVbkPopTx),
	}
	defaultPopTxVbkBlock = VbkBlock{
		4917,
		2,
		parse12Bytes("a793c872d6f6460e90bed623"),
		parse9Bytes("42bb968195f8c515d3"),
		parse9Bytes("eed7277a09efac4be9"),
		parse16Bytes("9f95f0a15628b06ba3b44c0190b5c049"),
		1553697485,
		117556515,
		1589362332,
	}
	btcTxBytes  = parseHex("01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f400000000")
	defaultPath = MerklePath{
		1659,
		parse32Bytes("94E097B110BA3ADBB7B6C4C599D31D675DE7BE6E722407410C08EF352BE585F1"),
		[][32]byte{
			parse32Bytes("4D66077FDF24246FFD6B6979DFEDEF5D46588654ADDEB35EDB11E993C131F612"),
			parse32Bytes("023D1ABE8758C6F917EC0C65674BBD43D66EE14DC667B3117DFC44690C6F5AF1"),
			parse32Bytes("096DDBA03CA952AF133FB06307C24171E53BF50AB76F1EDEABDE5E99F78D4EAD"),
			parse32Bytes("2F32CF1BEE50349D56FC1943AF84F2D2ABDA520F64DC4DB37B2F3DB20B0ECB57"),
			parse32Bytes("93E70120F1B539D0C1495B368061129F30D35F9E436F32D69967AE86031A2756"),
			parse32Bytes("F554378A116E2142F9F6315A38B19BD8A1B2E6DC31201F2D37A058F03C39C06C"),
			parse32Bytes("0824705685CECA003C95140434EE9D8BBBF4474B83FD4ECC2766137DB9A44D74"),
			parse32Bytes("B7B9E52F3EE8CE4FBB8BE7D6CF66D33A20293F806C69385136662A74453FB162"),
			parse32Bytes("1732C9A35E80D4796BABEA76AACE50B49F6079EA3E349F026B4491CFE720AD17"),
			parse32Bytes("2D9B57E92AB51FE28A587050FD82ABB30ABD699A5CE8B54E7CD49B2A827BCB99"),
			parse32Bytes("DCBA229ACDC6B7F028BA756FD5ABBFEBD31B4227CD4137D728EC5EA56C457618"),
			parse32Bytes("2CF1439A6DBCC1A35E96574BDDBF2C5DB9174AF5AD0D278FE92E06E4AC349A42"),
		},
	}
	defaultPopTxBtcBlock = BtcBlock{
		549453824,
		parse32Bytes("134f09d43659eb53982d9afb444b96fa4bb58c037d2914000000000000000000"),
		parse32Bytes("ce0b1a9a77dd0db127b5df4bc368cd6ac299a9747d991ec2dacbc0b699a2e4a5"),
		1553699251,
		388767596,
		3225120883,
	}
	btcBlock1 = BtcBlock{
		545259520,
		parse32Bytes("fc61cc9d4eac4b2d14761a4d06af8a9ef073dcd7fb5e0d000000000000000000"),
		parse32Bytes("a31508d4b101d0ad11e43ef9419c23fc277f67edae83c598ee70866dbcef5e25"),
		1553697574,
		388767596,
		2943621345,
	}
	btcBlock2 = BtcBlock{
		541065216,
		parse32Bytes("3f8e3980304439d853c302f6e496285e110e2512515313000000000000000000"),
		parse32Bytes("39a72c22268381bd8d9dcfe002f472634a24cf0454de8b50f89e10891e5ffb1d"),
		1553698272,
		388767596,
		2450139460,
	}
	btcBlock3 = BtcBlock{
		536870912,
		parse32Bytes("baa42e40345a7f826a31d37db1a5d64b67b72732477422000000000000000000"),
		parse32Bytes("a33ad6be0634647b26633ab85fa8de258480bbb25e59c68e48bb0b608b12362b"),
		1553699088,
		388767596,
		4040279113,
	}
	defaultSignature       = parseHex("3045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7")
	defaultPublicKey       = parseHex("3056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df99")
	defaultVbkPopTxAddress = addressFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX")
	defaultVbkPopTx        = VbkPopTx{
		networkByte,
		defaultVbkPopTxAddress,
		defaultPopTxVbkBlock,
		BtcTx{btcTxBytes},
		defaultPath,
		defaultPopTxBtcBlock,
		[]BtcBlock{btcBlock1, btcBlock2, btcBlock3},
		defaultSignature,
		defaultPublicKey,
	}
	defaultVbkPopTxEncoded = "02046102011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2641000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c523005ebbbe9c02011b01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f40000000002019f040000067b040000000c040000000400000020204d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c02000000000000000000014297d038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d74a999c26acd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc0010350000080200000000000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eefbc6d8670ee98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17e11874af500000402000000000000000000013535112250e115e2896e4f602c353d839443080398e3f1dfb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e08d9b5c6c1f2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17dd3316a827f5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b643406bed63aa310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df99"
)

func TestVbkPopTxDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	vbktx := parseHex(defaultVbkPopTxEncoded)
	stream := bytes.NewReader(vbktx)
	decoded := VbkPopTx{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultVbkPopTx.NetworkOrType.TypeID, decoded.NetworkOrType.TypeID)
	assert.Equal(defaultVbkPopTx.Address, decoded.Address)
	assert.Equal(defaultVbkPopTx.PublishedBlock.Height, decoded.PublishedBlock.Height)
	assert.Equal(defaultVbkPopTx.BitcoinTransaction.Tx, decoded.BitcoinTransaction.Tx)
	assert.Equal(defaultVbkPopTx.BlockOfProof.Version, decoded.BlockOfProof.Version)
	assert.Equal(len(defaultVbkPopTx.BlockOfProofContext), len(decoded.BlockOfProofContext))
	assert.Equal(defaultVbkPopTx.BlockOfProofContext[0].Version, decoded.BlockOfProofContext[0].Version)
	assert.Equal(defaultVbkPopTx.MerklePath.Index, decoded.MerklePath.Index)
	assert.Equal(defaultVbkPopTx.Signature, decoded.Signature)
	assert.Equal(defaultVbkPopTx.PublicKey, decoded.PublicKey)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestVbkPopTxSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultVbkPopTx.ToVbkEncoding(stream))
	assert.Equal(defaultVbkPopTxEncoded, hex.EncodeToString(stream.Bytes()))
}

func TestVbkPopTxRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	txEncoded := parseHex(defaultVbkPopTxEncoded)
	stream := bytes.NewReader(txEncoded)
	decoded := VbkPopTx{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultVbkPopTxAddress, decoded.Address)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	assert.Equal(defaultVbkPopTxEncoded, hex.EncodeToString(outputStream.Bytes()))
}
