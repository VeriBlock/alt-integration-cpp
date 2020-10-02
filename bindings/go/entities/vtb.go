package entities

// Vtb ...
type Vtb struct {
	version         uint32
	transaction     VbkPopTx
	merklePath      VbkMerklePath
	containingBlock VbkBlock
}
