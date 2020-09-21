package base

import (
	"encoding/binary"
	"io"
	"math/big"
	"reflect"
	"unsafe"
)

// TrimmedArray - Converts the input to the byte array and
// trims it's size to the lowest possible value
func TrimmedArray(input int64) []byte {
	x := unsafe.Sizeof(input)
	for ok := true; ok; ok = (x > 1) {
		if (input >> ((x - 1) * 8)) != 0 {
			break
		}
		x--
	}
	output := make([]byte, x)
	for i := uintptr(0); i < x; i++ {
		output[x-i-1] = byte(input)
		input >>= 8
	}
	return output
}

// WriteSingleBEValue ...
func WriteSingleBEValue(w io.Writer, value int64) error {
	dataBytes := TrimmedArray(value)
	err := binary.Write(w, binary.BigEndian, byte(binary.Size(dataBytes)))
	if err != nil {
		return err
	}
	_, err = w.Write(dataBytes)
	return err
}

// WriteContainer ...
func WriteContainer(w io.Writer, t interface{}, f func(w io.Writer, t interface{}) error) error {
	err := WriteSingleBEValue(w, int64(binary.Size(t)))
	if err != nil {
		return err
	}
	// TODO: Maybe not optimal solution. Get rid of reflect
	s := reflect.ValueOf(t)
	if s.Kind() != reflect.Slice {
		panic("InterfaceSlice() given a non-slice type")
	}
	for i := 0; i < s.Len(); i++ {
		v := s.Index(i).Interface()
		err = f(w, v)
		if err != nil {
			return err
		}
	}
	return nil
}

// WriteArrayOf ...
func WriteArrayOf(w io.Writer, t interface{}, f func(w io.Writer, t interface{}) error) error {
	return WriteContainer(w, t, f)
}

// WriteSingleByteLenValue ...
func WriteSingleByteLenValue(w io.Writer, value interface{}) error {
	// fmt.Printf("Val: %v\n", value.(int))
	// num := int64(binary.Size([]int{999999991, 999999991, 999999991}))
	// fmt.Printf("Val: %v\n", num)
	// CheckRange(num, 0, math.MaxUint8)
	err := binary.Write(w, binary.BigEndian, byte(binary.Size(value)))
	if err != nil {
		return err
	}
	_, err = w.Write(U256Bytes(value.(*big.Int)))
	return err
}
