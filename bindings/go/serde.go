package veriblock

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"io"
	"math"
	"reflect"
	"unsafe"
)

// Parse - parses hex string to byte array.
func Parse(src string) []byte {
	res, _ := hex.DecodeString(src)
	return res
}

// CheckRangePanic - Checks if expression 'min' <= 'num' <= 'max' is true. If false, panics.
func CheckRangePanic(num, min, max int64) {
	if num < min {
		panic("value is less than minimal")
	}
	if num > max {
		panic("value is greater than maximum")
	}
}

// CheckRange - Checks if expression 'min' <= 'num' <= 'max' is true. If false, returns error.
func CheckRange(num, min, max int64) error {
	if num < min {
		return errors.New("value is less than minimal")
	}
	if num > max {
		return errors.New("value is greater than maximum")
	}
	return nil
}

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

// WriteArrayOf ...
func WriteArrayOf(w io.Writer, t interface{}, f func(w io.Writer, t interface{}) error) error {
	ref := reflect.ValueOf(t)
	if ref.Kind() != reflect.Slice {
		panic("InterfaceSlice() given a non-slice type")
	}
	err := WriteSingleBEValue(w, int64(ref.Len()))
	if err != nil {
		return err
	}
	for i := 0; i < ref.Len(); i++ {
		val := ref.Index(i).Interface()
		err = f(w, val)
		if err != nil {
			return err
		}
	}
	return nil
}

// WriteSingleByteLenValue ...
func WriteSingleByteLenValue(w io.Writer, value interface{}) error {
	size := int64(binary.Size(value))
	err := CheckRange(size, 0, math.MaxUint8)
	if err != nil {
		return err
	}
	err = binary.Write(w, binary.BigEndian, byte(binary.Size(value)))
	if err != nil {
		return err
	}
	return binary.Write(w, binary.BigEndian, value)
}

func pad(src []byte, size int) []byte {
	r := make([]byte, size)
	s := size - len(src)
	if s < 0 {
		panic("Size less than zero")
	}
	copy(r[s:], src)
	return r
}

// ReadSingleByteLenValue ...
func ReadSingleByteLenValue(r io.Reader, minLen, maxLen int32) ([]byte, error) {
	var length byte
	err := binary.Read(r, binary.BigEndian, &length)
	if err != nil {
		return nil, err
	}
	err = CheckRange(int64(length), int64(minLen), int64(maxLen))
	if err != nil {
		return nil, err
	}
	buf := make([]byte, length)
	_, err = r.Read(buf)
	if err != nil {
		return nil, err
	}
	return buf, nil
}

func readSingleBEValue(r io.Reader) (uint32, error) {
	data, err := ReadSingleByteLenValue(r, 0, 4)
	if err != nil {
		return 0, err
	}
	padded := pad(data, 4)
	dataStream := bytes.NewReader(padded)
	var buf uint32
	err = binary.Read(dataStream, binary.BigEndian, &buf)
	if err != nil {
		return 0, err
	}
	return buf, nil
}

// ReadArrayOf ...
func ReadArrayOf(r io.Reader, readFunc func(r io.Reader) (interface{}, error)) ([]interface{}, error) {
	count, err := readSingleBEValue(r)
	if err != nil {
		return nil, err
	}
	err = CheckRange(int64(count), 0, int64(math.MaxInt32))
	if err != nil {
		return nil, err
	}
	items := make([]interface{}, count)
	for i := 0; i < int(count); i++ {
		res, err := readFunc(r)
		if err != nil {
			return nil, err
		}
		items[i] = res
	}
	return items, nil
}

// ReadArrayOfFunc ...
func ReadArrayOfFunc(stream io.Reader) (interface{}, error) {
	return ReadSingleByteLenValue(stream, 0, math.MaxInt32)
}

// readVarLenValue
// readNetworkByte
