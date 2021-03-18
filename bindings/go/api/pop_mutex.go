package api

import "sync"

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

func (m *SafeMutex) IsLocked() bool {
	return m.locked
}

func (m *SafeMutex) Unlock() {
	m.mu.Unlock()
	m.locked = false
}
