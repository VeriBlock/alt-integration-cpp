// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"sync"
)

type SafeMutex struct {
	mu     *sync.Mutex
	locked bool
}

func NewSafeMutex() *SafeMutex {
	return &SafeMutex{new(sync.Mutex), false}
}

func (m *SafeMutex) Lock() {
	m.mu.Lock()
	m.locked = true
}

func (m *SafeMutex) AssertMutexLocked(str string) {
	if !m.locked {
		panic(str)
	}
}

func (m *SafeMutex) Unlock() {
	m.locked = false
	m.mu.Unlock()
}
