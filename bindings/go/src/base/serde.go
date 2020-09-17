package base

import "unsafe"

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

// std::vector<uint8_t> trimmedArray(int64_t input) {
// 	size_t x = sizeof(int64_t);
// 	do {
// 	  if ((input >> ((x - 1) * 8)) != 0) {
// 		break;
// 	  }
// 	  x--;
// 	} while (x > 1);

// 	std::vector<uint8_t> output(x);
// 	for (size_t i = 0; i < x; i++) {
// 	  output[x - i - 1] = (uint8_t)input;
// 	  input >>= 8;
// 	}

// 	return output;
//   }

// func ReadDouble(stream *ReadStream) float64 {
// 	uint64_t d = stream.readBE<uint64_t>();
// 	double r = 0;
// 	memcpy(&r, &d, sizeof(d));
// 	return r;
// }
