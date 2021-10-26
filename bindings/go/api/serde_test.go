// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultAltBlockEncoded        = "201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c201aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000005ba0000009c"
	defaultVbkBlockEncoded        = "41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
	defaultBtcBlockEncoded        = "500000002000000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def75e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f82213b75c841a011a2e00f29a"
	defaultVtbEncoded             = "0000000102046102011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2641000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c523005ebbbe9c02011b01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f40000000002019f040000067b040000000c040000000400000020204d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c02000000000000000000014297d038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d74a999c26acd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc0010350000080200000000000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eefbc6d8670ee98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17e11874af500000402000000000000000000013535112250e115e2896e4f602c353d839443080398e3f1dfb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e08d9b5c6c1f2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17dd3316a827f5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b643406bed63aa310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df990400000000040000000d202a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec60400000006205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f59693997872020d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c06fe913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350eeb8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e902000000000000000000000000000000000000000000000000000000000000000002036252dfc621de420fb083ad9d8767cba627eddeec64e421e9576cee21297dd0a41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
	defaultAtvEncoded             = "0000000101580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000000220000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000041000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
	defaultPublicationDataEncoded = "0100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f206279746573"
	defaultPopDataEncoded         = "00000001010141000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f301010000000000000101010000000102046102011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2641000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c523005ebbbe9c02011b01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f40000000002019f040000067b040000000c040000000400000020204d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c02000000000000000000014297d038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d74a999c26acd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc0010350000080200000000000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eefbc6d8670ee98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17e11874af500000402000000000000000000013535112250e115e2896e4f602c353d839443080398e3f1dfb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e08d9b5c6c1f2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17dd3316a827f5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b643406bed63aa310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df990400000000040000000d202a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec60400000006205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f59693997872020d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c06fe913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350eeb8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e902000000000000000000000000000000000000000000000000000000000000000002036252dfc621de420fb083ad9d8767cba627eddeec64e421e9576cee21297dd0a41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f301010000000000000101010000000101580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000000220000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000041000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
)

func assertHexDecode(str string) []byte {
	res, err := hex.DecodeString(str)
	if err != nil {
		panic(err)
	}
	return res
}

func TestAltBlockSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultAltBlockEncoded)

	block := NewAltBlock([]byte{}, []byte{}, 0, 0)
	err := block.DeserializeFromVbkAltBlock(encodedBytes)
	assert.NoError(err)
	assert.NotNil(block)

	assert.Equal(block.GetHeight(), int32(1466))
	assert.Equal(block.GetTimestamp(), uint32(156))
	assert.Equal(block.GetHash(), assertHexDecode("1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"))
	assert.Equal(block.GetPreviousBlock(), assertHexDecode("1aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"))

	assert.Equal(block.SerializeToVbk(), encodedBytes)

	err = block.DeserializeFromVbkAltBlock([]byte{1, 2, 3, 4})
	assert.Error(err)
}

func TestVbkBlockSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultVbkBlockEncoded)

	vbkBlock := CreateVbkBlock()
	err := vbkBlock.DeserializeFromVbk(encodedBytes)
	assert.NoError(err)
	assert.NotNil(vbkBlock)

	assert.Equal(vbkBlock.GetHeight(), int32(5000))
	assert.Equal(vbkBlock.GetVersion(), int16(2))
	assert.Equal(vbkBlock.GetDifficulty(), int32(16842752))
	assert.Equal(vbkBlock.GetNonce(), uint64(1))
	assert.Equal(vbkBlock.GetTimestamp(), uint32(1553699059))
	assert.Equal(vbkBlock.GetPreviousBlock(), assertHexDecode("449c60619294546ad825af03"))
	assert.Equal(vbkBlock.GetPreviousKeystone(), assertHexDecode("b0935637860679ddd5"))
	assert.Equal(vbkBlock.GetSecondPreviousKeystone(), assertHexDecode("5ee4fd21082e18686e"))
	assert.Equal(vbkBlock.GetMerkleRoot(), assertHexDecode("26bbfda7d5e4462ef24ae02d67e47d78"))

	assert.Equal(vbkBlock.SerializeToVbk(), encodedBytes)

	err = vbkBlock.DeserializeFromVbk([]byte{1, 2, 3, 4})
	assert.Error(err)
}

func TestBtcBlockSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultBtcBlockEncoded)

	btcBlock := CreateBtcBlock()
	err := btcBlock.DeserializeFromVbk(encodedBytes)
	assert.NoError(err)
	assert.NotNil(btcBlock)

	assert.Equal(btcBlock.GetVersion(), uint32(536870912))
	assert.Equal(btcBlock.GetTimestamp(), uint32(1555501858))
	assert.Equal(btcBlock.GetDifficulty(), uint32(436279940))
	assert.Equal(btcBlock.GetNonce(), uint32(0x9af2002e))
	assert.Equal(btcBlock.GetPreviousBlock(), assertHexDecode("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"))
	assert.Equal(btcBlock.GetMerkleRoot(), assertHexDecode("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"))

	assert.Equal(btcBlock.SerializeToVbk(), encodedBytes)

	err = btcBlock.DeserializeFromVbk([]byte{1, 2, 3, 4})
	assert.Error(err)
}

func TestVtbSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultVtbEncoded)

	vbkBlock := CreateVbkBlock()
	err := vbkBlock.DeserializeFromVbk(assertHexDecode(defaultVbkBlockEncoded))
	assert.NoError(err)
	assert.NotNil(vbkBlock)

	vtb := CreateVtb()
	err = vtb.DeserializeFromVbk(encodedBytes)
	assert.NoError(err)
	assert.NotNil(vtb)

	vtb.GetContainingBlock().assertEquals(assert, vbkBlock)

	assert.Equal(vtb.SerializeToVbk(), encodedBytes)

	err = vtb.DeserializeFromVbk([]byte{1, 2, 3, 4})
	assert.Error(err)
}

func TestAtvSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultAtvEncoded)

	vbkBlock := CreateVbkBlock()
	err := vbkBlock.DeserializeFromVbk(assertHexDecode(defaultVbkBlockEncoded))
	assert.NoError(err)
	assert.NotNil(vbkBlock)

	atv := CreateAtv()
	err = atv.DeserializeFromVbk(encodedBytes)
	assert.NoError(err)
	assert.NotNil(atv)

	atv.GetBlockOfProof().assertEquals(assert, vbkBlock)

	assert.Equal(atv.SerializeToVbk(), encodedBytes)

	err = atv.DeserializeFromVbk([]byte{1, 2, 3, 4})
	assert.Error(err)
}

func TestPublicationDataSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultPublicationDataEncoded)

	publicationData := CreatePublicationData()
	err := publicationData.DeserializeFromVbk(encodedBytes)
	assert.NoError(err)
	assert.NotNil(publicationData)

	assert.Equal(publicationData.SerializeToVbk(), encodedBytes)

	err = publicationData.DeserializeFromVbk([]byte{1, 2, 3, 4})
	assert.Error(err)
}

func TestPopDataSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultPopDataEncoded)

	atv := CreateAtv()
	err := atv.DeserializeFromVbk(assertHexDecode(defaultAtvEncoded))
	assert.NoError(err)
	assert.NotNil(atv)

	vtb := CreateVtb()
	err = vtb.DeserializeFromVbk(assertHexDecode(defaultVtbEncoded))
	assert.NoError(err)
	assert.NotNil(vtb)

	vbkBlock := CreateVbkBlock()
	err = vbkBlock.DeserializeFromVbk(assertHexDecode(defaultVbkBlockEncoded))
	assert.NoError(err)
	assert.NotNil(vbkBlock)

	popData := CreatePopData()
	err = popData.DeserializeFromVbk(encodedBytes)
	assert.NoError(err)
	assert.NotNil(popData)

	assert.Equal(len(popData.GetAtvs()), 1)
	assert.Equal(len(popData.GetVtbs()), 1)
	assert.Equal(len(popData.GetContext()), 1)

	popData.GetAtvs()[0].assertEquals(assert, atv)
	popData.GetVtbs()[0].assertEquals(assert, vtb)
	popData.GetContext()[0].assertEquals(assert, vbkBlock)

	assert.Equal(popData.SerializeToVbk(), encodedBytes)

	err = popData.DeserializeFromVbk([]byte{1, 2, 3, 4})
	assert.Error(err)
}
