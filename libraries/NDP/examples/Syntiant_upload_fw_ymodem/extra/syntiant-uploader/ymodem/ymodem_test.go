package ymodem

import (
	"log"
	"testing"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

func TestYMODEM(t *testing.T) {
	log.SetOutput(GinkgoWriter)
	RegisterFailHandler(Fail)
	RunSpecs(t, "YMODEM")
}

var _ = Describe("YMODEM", func() {
	It("should calculate the CRC16 correctly", func() {
		data := []byte{72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100}
		Ω(CRC16(data)).Should(Equal(uint16(39210)))
	})

	It("should calculate the CRC16 with a constant correclty", func() {
		data := []byte{72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100}
		Ω(CRC16Constant(data, 13)).Should(Equal(uint16(43803)))
	})
})
