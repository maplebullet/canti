// +build !gui

package gui

type GUI struct{}

func NewGUI() *GUI {
	return &GUI{}
}

func (g *GUI) Start() {
	// Stub - not implemented without gui build tag
}
