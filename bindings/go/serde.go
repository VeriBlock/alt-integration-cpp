package veriblock

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"math"
	"reflect"
	"unsafe"
)

// WithName - Can return a name of item.
type WithName interface {
	Name() string
}

// Serde - Serializes struct to Vbk Encoded bytes and deserializes Vbk Encoded bytes to struct.
type Serde interface {
	ToVbkEncoding(stream io.Writer) error
	FromVbkEncoding(stream io.Reader) error
}

// SerdeRaw - Serializes struct to bytes and deserializes bytes to struct.
type SerdeRaw interface {
	ToRaw(stream io.Writer) error
	FromRaw(stream io.Reader) error
}

// ReverseBytes - reverses bytes.
func ReverseBytes(src []byte) []byte {
	newBytes := make([]byte, len(src))
	for i, j := 0, len(src)-1; i <= j; i, j = i+1, j-1 {
		newBytes[i], newBytes[j] = src[j], src[i]
	}
	return newBytes
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

// Pad - returns padded bytes.
func Pad(src []byte, size int) []byte {
	r := make([]byte, size)
	s := size - len(src)
	if s < 0 {
		panic("Size less than zero")
	}
	copy(r[s:], src)
	return r
}

// FixedArray - converts the input to the byte array
func FixedArray(input interface{}) ([]byte, error) {
	inputStream := new(bytes.Buffer)
	err := binary.Write(inputStream, binary.BigEndian, input)
	if err != nil {
		return nil, err
	}
	return inputStream.Bytes(), nil
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

// WriteVarLenValue ...
func WriteVarLenValue(stream io.Writer, value []byte) error {
	WriteSingleBEValue(stream, int64(len(value)))
	_, err := stream.Write(value)
	return err
}

// WriteSingleFixedBEValue ...
func WriteSingleFixedBEValue(stream io.Writer, value interface{}) error {
	dataStream := new(bytes.Buffer)
	err := binary.Write(dataStream, binary.BigEndian, value)
	if err != nil {
		return err
	}
	return WriteSingleByteLenValue(stream, dataStream.Bytes())
}

// ReadSingleByteLenValue ...
func ReadSingleByteLenValue(stream io.Reader, minLen, maxLen int32) ([]byte, error) {
	var length byte
	err := binary.Read(stream, binary.BigEndian, &length)
	if err != nil {
		return nil, err
	}
	err = CheckRange(int64(length), int64(minLen), int64(maxLen))
	if err != nil {
		return nil, err
	}
	buf := make([]byte, length)
	_, err = stream.Read(buf)
	if err != nil {
		return nil, err
	}
	return buf, nil
}

// ReadSingleByteLenValueDefault ...
func ReadSingleByteLenValueDefault(stream io.Reader) ([]byte, error) {
	return ReadSingleByteLenValue(stream, 0, math.MaxInt32)
}

// ReadSingleBEValue ...
func ReadSingleBEValue(r io.Reader, buf interface{}) error {
	num := binary.Size(buf)
	data, err := ReadSingleByteLenValue(r, 0, int32(num))
	if err != nil {
		return err
	}
	padded := Pad(data, int(num))
	dataStream := bytes.NewReader(padded)
	return binary.Read(dataStream, binary.BigEndian, buf)
}

// ReadArrayOf ...
func ReadArrayOf(r io.Reader, min, max int64, readFunc func(r io.Reader) (interface{}, error)) ([]interface{}, error) {
	var count uint32
	err := ReadSingleBEValue(r, &count)
	if err != nil {
		return nil, err
	}
	err = CheckRange(int64(count), min, max)
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

// ReadVarLenValue ...
func ReadVarLenValue(stream io.Reader, minLen, maxLen int64) ([]byte, error) {
	var length int32
	err := ReadSingleBEValue(stream, &length)
	if err != nil {
		return nil, err
	}
	err = CheckRange(int64(length), minLen, maxLen)
	if err != nil {
		return nil, err
	}
	if length == 0 {
		return []byte{}, nil
	}
	buf := make([]byte, length)
	_, err = stream.Read(buf)
	return buf, err
}

// UnmarshalJSON parses a hash in hex syntax.
func UnmarshalJSON(input, out []byte) error {
	if !isString(input) {
		return fmt.Errorf("non-string")
	}
	return UnmarshalFixedText(input[1:len(input)-1], out)
}

// UnmarshalFixedText decodes the input as a string with 0x prefix. The length of out
// determines the required input length. This function is commonly used to implement the
// UnmarshalText method for fixed-size types.
func UnmarshalFixedText(input, out []byte) error {
	raw, err := checkText(input, true)
	if err != nil {
		return err
	}
	if len(raw)/2 != len(out) {
		return fmt.Errorf("hex string has length %d, want %d", len(raw), len(out)*2)
	}
	// Pre-verify syntax before modifying out.
	for _, b := range raw {
		if decodeNibble(b) == badNibble {
			return fmt.Errorf("invalid hex string")
		}
	}
	hex.Decode(out, raw)
	return nil
}

func isString(input []byte) bool {
	return len(input) >= 2 && input[0] == '"' && input[len(input)-1] == '"'
}

const badNibble = ^uint64(0)

func decodeNibble(in byte) uint64 {
	switch {
	case in >= '0' && in <= '9':
		return uint64(in - '0')
	case in >= 'A' && in <= 'F':
		return uint64(in - 'A' + 10)
	case in >= 'a' && in <= 'f':
		return uint64(in - 'a' + 10)
	default:
		return badNibble
	}
}

func checkText(input []byte, wantPrefix bool) ([]byte, error) {
	if len(input) == 0 {
		return nil, nil // empty strings are allowed
	}
	if bytesHave0xPrefix(input) {
		input = input[2:]
	} else if wantPrefix {
		return nil, fmt.Errorf("hex string without 0x prefix")
	}
	if len(input)%2 != 0 {
		return nil, fmt.Errorf("hex string of odd length")
	}
	return input, nil
}

func bytesHave0xPrefix(input []byte) bool {
	return len(input) >= 2 && input[0] == '0' && (input[1] == 'x' || input[1] == 'X')
}
