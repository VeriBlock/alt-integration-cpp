package api

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
