package base

import "errors"

// CheckRange - Checks if expression 'min' <= 'num' <= 'max' is true. If false, panics.
func CheckRange(num, min, max int64) {
	if num < min {
		panic("value is less than minimal")
	}
	if num > max {
		panic("value is greater than maximum")
	}
}

// CheckRangeWithErr - Checks if expression 'min' <= 'num' <= 'max' is true. If false, returns error.
func CheckRangeWithErr(num, min, max int64) error {
	if num < min {
		return errors.New("value is less than minimal")
	}
	if num > max {
		return errors.New("value is greater than maximum")
	}
	return nil
}
