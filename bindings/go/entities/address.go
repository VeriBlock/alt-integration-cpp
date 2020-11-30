package entities

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// AddressType ...
type AddressType uint8

const (
	// AddressTypeZeroUnused ..
	AddressTypeZeroUnused AddressType = iota
	// AddressTypeStandard ..
	AddressTypeStandard
	// AddressTypeProofOfProof ..
	AddressTypeProofOfProof
	// AddressTypeMultisig ..
	AddressTypeMultisig
)

// Consts
const (
	StartingChar       = 'V'
	MultisigEndingChar = '0'

	MultisigAddressMValue      = 1
	MultisigAddressNValue      = 2
	MultisigAddressMinNValue   = 2
	MultisigAddressMaxNValue   = 58
	MultisigAddressMaxMValue   = 58
	MultisigAddressDataEnd     = 24
	MultisigAddressChecksumEnd = 28
)

// Address ...
type Address struct {
	addressType AddressType
	address     string
}

func getDataPortionFromAddress(address string) (string, error) {
	if len(address) != veriblock.AddressSize {
		return "", errors.New("Invalid address size")
	}
	return address[0 : MultisigAddressDataEnd+1], nil
}

func isMultisig(address string) (bool, error) {
	if len(address) != veriblock.AddressSize {
		return false, errors.New("Invalid address size")
	}
	return (address[veriblock.AddressSize-1] == MultisigEndingChar), nil
}

func getChecksumPortionFromAddress(address string, multisig bool) (string, error) {
	if len(address) != veriblock.AddressSize {
		return "", errors.New("Invalid address size")
	}
	if multisig {
		return address[MultisigAddressDataEnd+1 : MultisigAddressChecksumEnd+1], nil
	}
	return address[MultisigAddressDataEnd+1:], nil
}

func isBase58String(input string) bool {
	_, err := veriblock.DecodeBase58(input)
	return err == nil
}

func isBase59String(input string) bool {
	_, err := veriblock.DecodeBase59(input)
	return err == nil
}

func calculateChecksum(data string, multisig bool) string {
	hash := sha256.Sum256([]byte(data))
	checksum := veriblock.EncodeBase58(hash[:])
	if multisig {
		return checksum[0 : 3+1]
	}
	return checksum[0 : 4+1]
}

func addressChecksum(address Address) string {
	return calculateChecksum(address.ToString(), address.GetType() == AddressTypeMultisig)
}

// FromPublicKey ...
func (v *Address) FromPublicKey(publicKey []byte) {
	keyHash := sha256.Sum256(publicKey)
	keyHashEncoded := veriblock.EncodeBase58(keyHash[:])
	data := "V" + keyHashEncoded[0:MultisigAddressDataEnd]
	checksum := calculateChecksum(data, false)
	*v = Address{AddressTypeStandard, data + checksum}
}

// IsDerivedFromPublicKey ...
func (v *Address) IsDerivedFromPublicKey(publicKey []byte) bool {
	expectedAddress := Address{}
	expectedAddress.FromPublicKey(publicKey)
	if *v != expectedAddress {
		return false
	}
	if addressChecksum(*v) != addressChecksum(expectedAddress) {
		return false
	}
	return true
}

// FromVbkEncoding ...
func (v *Address) FromVbkEncoding(stream io.Reader) error {
	var addressType uint8
	if err := binary.Read(stream, binary.LittleEndian, &addressType); err != nil {
		return err
	}
	addressBytes, err := veriblock.ReadSingleByteLenValue(stream, 0, veriblock.AddressSize)
	if err != nil {
		return err
	}
	var addressText string
	switch AddressType(addressType) {
	case AddressTypeStandard:
		addressText = veriblock.EncodeBase58(addressBytes)
		break
	case AddressTypeMultisig:
		addressText = veriblock.EncodeBase59(addressBytes)
		break
	default:
		return errors.New("invalid-address-type")
	}
	return v.FromString(addressText)
}

// FromVbkEncodingBytes ...
func (v *Address) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}

// FromString ...
func (v *Address) FromString(str string) error {
	address, err := NewAddress(str)
	if err != nil {
		return err
	}
	*v = *address
	return nil
}

// ToVbkEncoding ...
func (v *Address) ToVbkEncoding(stream io.Writer) error {
	binary.Write(stream, binary.BigEndian, byte(v.GetType()))
	var decoded []byte
	var err error
	switch v.GetType() {
	case AddressTypeStandard:
		decoded, err = veriblock.DecodeBase58(v.ToString())
		break
	case AddressTypeMultisig:
		decoded, err = veriblock.DecodeBase59(v.ToString())
		break
	default:
		// If we don't know address type, do not encode anything
		return errors.New("Unsupported address type")
	}
	if err != nil {
		return err
	}
	veriblock.WriteSingleByteLenValue(stream, decoded)
	return nil
}

// ToVbkEncodingBytes ...
func (v *Address) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// ToString ...
func (v *Address) ToString() string {
	return v.address
}

// GetType ...
func (v *Address) GetType() AddressType {
	return v.addressType
}

// NewAddress ...
func NewAddress(input string) (*Address, error) {
	if len(input) != veriblock.AddressSize {
		return nil, errors.New("isValidAddress(): invalid address length")
	}
	if input[0] != StartingChar {
		return nil, errors.New("isValidAddress(): not a valid VBK address")
	}
	data, err := getDataPortionFromAddress(input)
	if err != nil {
		return nil, err
	}
	multisig, err := isMultisig(input)
	if err != nil {
		return nil, err
	}
	checksum, err := getChecksumPortionFromAddress(input, multisig)
	if err != nil {
		return nil, err
	}
	if multisig {
		if !isBase59String(input) {
			return nil, errors.New("isValidAddress(): not a base59 string")
		}
		/* To make the addresses 'human-readable' we add 1 to the decoded value (1
		 * in Base58 is 0, but we want an address with a '1' in the m slot to
		 * represent m=1, for example). this allows addresses with m and n both <=
		 * 9 to be easily recognized. Additionally, an m or n value of 0 makes no
		 * sense, so this allows multisig to range from 1 to 58, rather than what
		 * would have otherwise been 0 to 57. */
		decodedM, err := veriblock.DecodeBase58(string(input[MultisigAddressMValue]))
		decodedN, err := veriblock.DecodeBase58(string(input[MultisigAddressNValue]))
		if err != nil {
			return nil, err
		}
		n := decodedN[0] + 1
		m := decodedM[0] + 1
		if n < MultisigAddressMinNValue {
			return nil, errors.New("isValidAddress(): not enough addresses to be multisig")
		}
		if m > n {
			return nil, errors.New("isValidAddress(): address has more signatures than addresses")
		}
		if (n > MultisigAddressMaxNValue) ||
			(m > MultisigAddressMaxMValue) {
			return nil, errors.New("isValidAddress(): too many addresses/signatures")
		}
		if !isBase58String(input[0 : veriblock.AddressSize-1]) {
			return nil, errors.New("isValidAddress(): remainder is not a base58 string")
		}
	} else {
		if !isBase58String(input) {
			return nil, errors.New("isValidAddress(): address is not a base58 string")
		}
	}
	expectedChecksum := calculateChecksum(data, multisig)
	if expectedChecksum != checksum {
		return nil, errors.New("isValidAddress(): checksum does not match")
	}
	addressType := AddressTypeStandard
	if multisig {
		addressType = AddressTypeMultisig
	}
	return &Address{addressType, input}, nil
}
