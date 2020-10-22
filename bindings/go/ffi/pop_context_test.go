package ffi

import "testing"

func TestPopContext(t *testing.T) {
	config := NewConfig()
	defer config.Free()
	if !config.SelectVbkParams("regtest", popvbkstartheight, &popvbkblocks) {
		t.Error("Failed to select btc params")
	}
	if !config.SelectBtcParams("regtest", popbtcstartheight, &popbtcblocks) {
		t.Error("Failed to select btc params")
	}
	OnGetAltchainID = func() int { return 1 }
	OnGetBootstrapBlock = func() string {
		// hash, _ := hex.DecodeString("1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c")
		// block := entities.AltBlock{
		// 	Hash:          hash,
		// 	PreviousBlock: []byte{},
		// 	Height:        156,
		// 	Timestamp:     1466,
		// }
		// stream := new(bytes.Buffer)
		// _ = block.ToVbkEncoding(stream)
		// return hex.EncodeToString(stream.Bytes())
		return "000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000000000000009c000005ba"
	}

	popContext := NewPopContext(&config)
	defer popContext.Free()
}
