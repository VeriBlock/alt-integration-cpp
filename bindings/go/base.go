// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package veriblock

import "github.com/eknkc/basex"

var base58, _ = basex.NewEncoding("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz")
var base59, _ = basex.NewEncoding("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz0")

// EncodeBase58 - encodes bytes to base58 string
func EncodeBase58(source []byte) string {
	return base58.Encode(source)
}

// DecodeBase58 - decodes base58 string to bytes
func DecodeBase58(source string) ([]byte, error) {
	return base58.Decode(source)
}

// EncodeBase59 - encodes bytes to base59 string
func EncodeBase59(source []byte) string {
	return base59.Encode(source)
}

// DecodeBase59 - decodes base59 string to bytes
func DecodeBase59(source string) ([]byte, error) {
	return base59.Decode(source)
}
