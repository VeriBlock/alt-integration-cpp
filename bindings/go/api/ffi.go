// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

type Payload interface {
	Name() string
	GetID() []byte
	ToJSON() (map[string]interface{}, error)
	SerializeToVbk() []byte
	DeserializeFromVbk(data []byte) error
}
