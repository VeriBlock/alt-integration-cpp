package base

// ReadStream ...
type ReadStream struct {
	pos    uint64
	buffer []byte
	size   uint64
}

// NewReadStream ...
func NewReadStream(buff interface{}, numOfBytes uint64) ReadStream {
	return ReadStream{
		pos:    0,
		buffer: nil, //*(*[]byte)(unsafe.Pointer(&buff)),
		size:   0,
	}
}

// Position ...
func (v *ReadStream) Position() uint64 { return v.pos }

// SetPosition ...
func (v *ReadStream) SetPosition(pos uint64) { v.pos = pos }

// Remaining ...
func (v *ReadStream) Remaining() uint64 { return v.size - v.pos }

// HasMore ...
func (v *ReadStream) HasMore(nbytes uint64) bool {
	return v.Remaining() >= nbytes
}

// Reset ...
func (v *ReadStream) Reset(pos uint64) { v.pos = 0 }
