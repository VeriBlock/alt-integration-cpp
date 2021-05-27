// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package veriblock

import (
	"bytes"
	"encoding/hex"
	"math"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestPad(t *testing.T) {
	t.Parallel()

	v := []byte{1, 2, 3, 4, 5}
	res := Pad(v, 8)
	if res[0] != 0 || len(res) != 8 {
		t.Fatalf("Invalid len: %v", res)
	}
}

func TestWriteSingleByteLenValue(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	v := []byte{1, 2, 3, 4, 5}
	w := new(bytes.Buffer)
	err := WriteSingleByteLenValue(w, v)
	if err != nil {
		t.Fatal(err)
	}
	res := hex.EncodeToString(w.Bytes())
	assert.Equal(res, "050102030405", "Wrong conversion result")
}

func TestSerde(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	arr := make([][32]byte, 3)
	res, _ := hex.DecodeString("01")
	copy(arr[0][:], res)
	res, _ = hex.DecodeString("02")
	copy(arr[1][:], res)
	res, _ = hex.DecodeString("03")
	copy(arr[2][:], res)

	w := new(bytes.Buffer)
	err := WriteArrayOf(w, arr, WriteSingleByteLenValue)
	if err != nil {
		t.Fatal(err)
	}

	r := bytes.NewReader(w.Bytes())
	actual, err := ReadArrayOf(r, 0, int64(math.MaxInt32), ReadArrayOfFunc)
	if err != nil {
		t.Fatal(err)
	}

	for i, arrItem := range arr {
		actualItem := actual[i].([]byte)
		for j := 0; j < len(arrItem); j++ {
			assert.Equal(arrItem[j], actualItem[j], "Actual array and expected array mismatch")
		}
	}
}
