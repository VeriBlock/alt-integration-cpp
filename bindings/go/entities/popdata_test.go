// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/json"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/stretchr/testify/assert"
)

var defaultPopDataEncoded = "00000001010141000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f301010000000000000101010000000102046102011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2641000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c523005ebbbe9c02011b01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f40000000002019f040000067b040000000c040000000400000020204d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c02000000000000000000014297d038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d74a999c26acd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc0010350000080200000000000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eefbc6d8670ee98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17e11874af500000402000000000000000000013535112250e115e2896e4f602c353d839443080398e3f1dfb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e08d9b5c6c1f2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17dd3316a827f5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b643406bed63aa310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df990400000000040000000d202a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec60400000006205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f59693997872020d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c06fe913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350eeb8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e902000000000000000000000000000000000000000000000000000000000000000002036252dfc621de420fb083ad9d8767cba627eddeec64e421e9576cee21297dd0a41000013700002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686eb53c1f4e259e6a0df23721a0b3b4b7ab5c9b9211070211cafff01c3f0101010000000101580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000000220000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000041000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
var defaultPopData = PopData{
	Version: 1,
	Context: []VbkBlock{{
		Height:                 5000,
		Version:                2,
		PreviousBlock:          parse12Bytes("449c60619294546ad825af03"),
		PreviousKeystone:       parse9Bytes("b0935637860679ddd5"),
		SecondPreviousKeystone: parse9Bytes("5ee4fd21082e18686e"),
		MerkleRoot:             parse16Bytes("26bbfda7d5e4462ef24ae02d67e47d78"),
		Timestamp:              1553699059,
		Difficulty:             16842752,
		Nonce:                  1,
	}},
	Vtbs: []Vtb{
		{
			Version: 1,
			Transaction: VbkPopTx{
				NetworkOrType: veriblock.NetworkBytePair{
					HasNetworkByte: false,
					NetworkByte:    0,
					TypeID:         uint8(veriblock.TxTypeVbkPopTx),
				},
				Address: addressFromString("VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"),
				PublishedBlock: VbkBlock{
					Height:                 4917,
					Version:                2,
					PreviousBlock:          parse12Bytes("a793c872d6f6460e90bed623"),
					PreviousKeystone:       parse9Bytes("42bb968195f8c515d3"),
					SecondPreviousKeystone: parse9Bytes("eed7277a09efac4be9"),
					MerkleRoot:             parse16Bytes("9f95f0a15628b06ba3b44c0190b5c049"),
					Timestamp:              1553697485,
					Difficulty:             117556515,
					Nonce:                  1589362332,
				},
				BitcoinTransaction: BtcTx{Tx: parseHex("01000000010ce74f1fb694a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df04216615cf92083f400000000")},
				MerklePath: MerklePath{
					Index:   1659,
					Subject: parse32Bytes("94E097B110BA3ADBB7B6C4C599D31D675DE7BE6E722407410C08EF352BE585F1"),
					Layers: [][32]byte{
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
				},
				BlockOfProof: BtcBlock{
					Version:       549453824,
					PreviousBlock: parse32Bytes("134f09d43659eb53982d9afb444b96fa4bb58c037d2914000000000000000000"),
					MerkleRoot:    parse32Bytes("ce0b1a9a77dd0db127b5df4bc368cd6ac299a9747d991ec2dacbc0b699a2e4a5"),
					Timestamp:     1553699251,
					Bits:          388767596,
					Nonce:         3225120883,
				},
				BlockOfProofContext: []BtcBlock{
					{
						Version:       545259520,
						PreviousBlock: parse32Bytes("fc61cc9d4eac4b2d14761a4d06af8a9ef073dcd7fb5e0d000000000000000000"),
						MerkleRoot:    parse32Bytes("a31508d4b101d0ad11e43ef9419c23fc277f67edae83c598ee70866dbcef5e25"),
						Timestamp:     1553697574,
						Bits:          388767596,
						Nonce:         2943621345,
					}, {
						Version:       541065216,
						PreviousBlock: parse32Bytes("3f8e3980304439d853c302f6e496285e110e2512515313000000000000000000"),
						MerkleRoot:    parse32Bytes("39a72c22268381bd8d9dcfe002f472634a24cf0454de8b50f89e10891e5ffb1d"),
						Timestamp:     1553698272,
						Bits:          388767596,
						Nonce:         2450139460,
					}, {
						Version:       536870912,
						PreviousBlock: parse32Bytes("baa42e40345a7f826a31d37db1a5d64b67b72732477422000000000000000000"),
						MerkleRoot:    parse32Bytes("a33ad6be0634647b26633ab85fa8de258480bbb25e59c68e48bb0b608b12362b"),
						Timestamp:     1553699088,
						Bits:          388767596,
						Nonce:         4040279113,
					}},
				Signature: parseHex("3045022100f4dce45edcc6bfc4a1f44ef04e47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c550f75d66b857a8fd9d75e7"),
				PublicKey: parseHex("3056301006072a8648ce3d020106052b8104000a03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df99"),
			},
			MerklePath: VbkMerklePath{
				TreeIndex: 0,
				Index:     13,
				Subject:   parse32Bytes("2a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec6"),
				Layers: [][32]byte{
					parse32Bytes("5b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f5969399787"),
					parse32Bytes("20d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae9"),
					parse32Bytes("c06fe913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c11"),
					parse32Bytes("049f68d350eeb8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e90"),
					parse32Bytes("0000000000000000000000000000000000000000000000000000000000000000"),
					parse32Bytes("36252dfc621de420fb083ad9d8767cba627eddeec64e421e9576cee21297dd0a"),
				},
			},
			ContainingBlock: VbkBlock{
				Height:                 4976,
				Version:                2,
				PreviousBlock:          parse12Bytes("449c60619294546ad825af03"),
				PreviousKeystone:       parse9Bytes("b0935637860679ddd5"),
				SecondPreviousKeystone: parse9Bytes("5ee4fd21082e18686e"),
				MerkleRoot:             parse16Bytes("b53c1f4e259e6a0df23721a0b3b4b7ab"),
				Timestamp:              1553699345,
				Difficulty:             117576138,
				Nonce:                  1099245043457,
			},
		},
	}, Atvs: []Atv{
		{
			Version: 1,
			Transaction: VbkTx{

				NetworkOrType: veriblock.NetworkBytePair{
					HasNetworkByte: false,
					NetworkByte:    0,
					TypeID:         uint8(veriblock.TxTypeVbkTx),
				},

				SourceAddress:  addressFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"),
				SourceAmount:   Coin{Units: 1000},
				Outputs:        []Output{},
				SignatureIndex: 7,
				PublicationData: PublicationData{
					Identifier:  0,
					Header:      []byte("header bytes"),
					PayoutInfo:  []byte("payout info bytes"),
					ContextInfo: []byte("context info bytes"),
				},
				Signature: parseHex("30440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7"),
				PublicKey: parseHex("3056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e"),
			},
			MerklePath: VbkMerklePath{
				TreeIndex: 1,
				Index:     0,
				Subject:   parse32Bytes("1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"),
				Layers: [][32]byte{
					parse32Bytes("0000000000000000000000000000000000000000000000000000000000000000"),
					parse32Bytes("0000000000000000000000000000000000000000000000000000000000000000"),
				},
			},
			BlockOfProof: VbkBlock{
				Height:                 5000,
				Version:                2,
				PreviousBlock:          parse12Bytes("449c60619294546ad825af03"),
				PreviousKeystone:       parse9Bytes("b0935637860679ddd5"),
				SecondPreviousKeystone: parse9Bytes("5ee4fd21082e18686e"),
				MerkleRoot:             parse16Bytes("26bbfda7d5e4462ef24ae02d67e47d78"),
				Timestamp:              1553699059,
				Difficulty:             16842752,
				Nonce:                  1,
			},
		},
	},
}

func TestPopDataDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	data := parseHex(defaultPopDataEncoded)
	decoded := PopData{}
	assert.NoError(decoded.FromVbkEncodingBytes(data))
	assert.Equal(decoded, defaultPopData)
}

func TestPopDataRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	atvBytes := parseHex(defaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	atv := Atv{}
	assert.NoError(atv.FromVbkEncoding(stream))

	vtbBytes := parseHex(defaultVtbEncoded)
	stream = bytes.NewReader(vtbBytes)
	vtb := Vtb{}
	assert.NoError(vtb.FromVbkEncoding(stream))

	expectedPopData := PopData{1, []VbkBlock{}, []Vtb{vtb}, []Atv{atv}}
	outputStream := new(bytes.Buffer)
	assert.NoError(expectedPopData.ToVbkEncoding(outputStream))

	decodedPopData := PopData{}
	assert.NoError(decodedPopData.FromVbkEncoding(outputStream))
	assert.Equal(expectedPopData, decodedPopData)
}

func TestPopDataToJson(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	defaultJSON := "{\"blockOfProof\":{\"difficulty\":16842752,\"hash\":\"2ba076219b4ff7ed36512275cd97599e23096ad42f119b5a\",\"height\":5000,\"id\":\"cd97599e23096ad42f119b5a\",\"merkleRoot\":\"26bbfda7d5e4462ef24ae02d67e47d78\",\"nonce\":1,\"previousBlock\":\"449c60619294546ad825af03\",\"previousKeystone\":\"b0935637860679ddd5\",\"secondPreviousKeystone\":\"5ee4fd21082e18686e\",\"timestamp\":1553699059,\"version\":2},\"id\":\"c6d96b8e87f3e347aa1d1051bb3af39c8ea60612ced905d11c6f92d7b6bd50f5\",\"merklePath\":{\"index\":0,\"layers\":[\"0000000000000000000000000000000000000000000000000000000000000000\",\"0000000000000000000000000000000000000000000000000000000000000000\"],\"subject\":\"1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c\",\"treeIndex\":1},\"transaction\":{\"hash\":\"0000000000000000000000000000000000000000000000000000000000000000\",\"networkByte\":0,\"outputs\":[],\"publicKey\":\"3056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e\",\"publicationData\":{\"contextInfo\":\"636f6e7465787420696e666f206279746573\",\"header\":\"686561646572206279746573\",\"identifier\":0,\"payoutInfo\":\"7061796f757420696e666f206279746573\"},\"signature\":\"30440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7\",\"signatureIndex\":7,\"sourceAddress\":\"V5Ujv72h4jEBcKnALGc4fKqs6CDAPX\",\"sourceAmount\":1000,\"type\":1},\"version\":1}"

	jsonmap, err := defaultAtv.ToJSON()
	assert.NoError(err)

	res, err := json.Marshal(jsonmap)
	assert.NoError(err)
	assert.Equal(defaultJSON, string(res))
}
