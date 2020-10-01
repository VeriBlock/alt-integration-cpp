package veriblock

import (
	"bytes"
	"encoding/hex"
	"testing"
)

func TestPad(t *testing.T) {
	v := []byte{1, 2, 3, 4, 5}
	res := pad(v, 8)
	if res[0] != 0 || len(res) != 8 {
		t.Fatalf("Invalid len: %v", res)
	}
}

func TestWriteSingleByteLenValue(t *testing.T) {
	v := []byte{1, 2, 3, 4, 5}
	w := new(bytes.Buffer)
	err := WriteSingleByteLenValue(w, v)
	if err != nil {
		t.Fatal(err)
	}
	res := hex.EncodeToString(w.Bytes())
	if res != "050102030405" {
		t.Fatalf("Wrong conversion result: %v", res)
	}
}

func TestSerde(t *testing.T) {
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
	actual, err := ReadArrayOf(r, ReadArrayOfFunc)
	if err != nil {
		t.Fatal(err)
	}

	for i, arrItem := range arr {
		actualItem := actual[i].([]byte)
		for j := 0; j < len(arrItem); j++ {
			if arrItem[j] != actualItem[j] {
				t.Fatal("Actual array and expected array mismatch")
			}
		}
	}
}
