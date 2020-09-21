package base

import (
	"bytes"
	"math/big"
	"testing"
)

func TestSerde(t *testing.T) {
	v := make([]*big.Int, 3)
	// v := make([]int, 3)

	ok := true
	v[0], ok = ParseBig256("01")
	v[1], ok = ParseBig256("02")
	v[2], ok = ParseBig256("03")
	if !ok {
		t.Fatal("Failed to parse big256")
	}
	// v[0] = 1
	// v[1] = 2
	// v[2] = 3

	w := new(bytes.Buffer)
	err := WriteArrayOf(w, v, WriteSingleByteLenValue)
	if err != nil {
		t.Fatal(err)
	}

	// w := new(w.Bytes)
	// 	ReadStream r(w.data());
	// std::vector<uint256> actual = readArrayOf<uint256>(r, [](ReadStream& R) -> uint256 {
	// 	return readSingleByteLenValue(R);
	// });

	// 	ASSERT_EQ(v, actual);

	t.Errorf("%v", v)
}
