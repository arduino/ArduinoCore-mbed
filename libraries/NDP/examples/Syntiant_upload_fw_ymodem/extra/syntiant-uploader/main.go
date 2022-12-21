package main

import (
	"io/ioutil"
	"log"
	"os"
	"strings"
	"time"

	"github.com/arduino/syntiant-uploader/ymodem"
	serial "github.com/facchinm/go-serial"
	"github.com/spf13/cobra"
)

var Port, Message, Wait string

func main() {
	log.SetFlags(log.LstdFlags | log.Lshortfile)

	mode := serial.Mode{BaudRate: 115200, Vtimeout: 100, Vmin: 0}

	var cmdSend = &cobra.Command{
		Use:   "send [port]",
		Short: "Send file",
		Long:  ``,
		Run: func(cmd *cobra.Command, args []string) {
			// Open connection
			connection, err := serial.OpenPort(Port, &mode)
			if err != nil {
				log.Fatalln(err)
			}

			time.Sleep(1 * time.Second)

			// Send initial message
			if len(Message) > 0 {
				if _, err := connection.Write([]byte(Message + "\r\n")); err != nil {
					log.Println(err)
				}
			}

			// Wait for message
			if len(Wait) > 0 {
				var result string
				buffer := make([]byte, 64)
				for {
					n, err := connection.Read(buffer)
					if err != nil {
						log.Fatalln(err)
					}
					if n == 0 {
						break
					}
					result += string(buffer[0:n])
					if strings.Contains(result, Wait) {
						break
					}
				}
			}

			// Open file
			fIn, err := os.Open(args[0])
			if err != nil {
				log.Fatalln(err)
			}

			data, err := ioutil.ReadAll(fIn)
			if err != nil {
				log.Fatalln(err)
			}

			fIn.Close()

			time.Sleep(1 * time.Second)

			// Send file
			if err := ymodem.ModemSend(connection, data, args[0]); err != nil {
				log.Fatalln(err)
			}

			log.Println(args[0], "sent successful")
		},
	}
	cmdSend.Flags().StringVarP(&Port, "port", "p", "", "serial port to connect to")
	cmdSend.Flags().StringVarP(&Message, "message", "m", "", "message to initiate data transfer")
	cmdSend.Flags().StringVarP(&Wait, "wait", "w", "", "message to wait before initiating data transfer")

	var cmdReceive = &cobra.Command{
		Use:   "receive [port]",
		Short: "Receive file",
		Long:  ``,
		Run: func(cmd *cobra.Command, args []string) {
			// Open connection
			connection, err := serial.OpenPort(Port, &mode)
			if err != nil {
				log.Fatalln(err)
			}

			time.Sleep(1 * time.Second)

			// Send initial message
			if len(Message) > 0 {
				if _, err := connection.Write([]byte(Message + "\r\n")); err != nil {
					log.Println(err)
				}
			}

			// Wait for message
			if len(Wait) > 0 {
				var result string
				buffer := make([]byte, 64)
				for {
					n, err := connection.Read(buffer)
					if err != nil {
						log.Fatalln(err)
					}
					if n == 0 {
						break
					}
					result += string(buffer[0:n])
					if strings.Contains(result, Wait) {
						break
					}
				}
			}

			// Receive file
			filename, data, err := ymodem.ModemReceive(connection)
			if err != nil {
				log.Fatalln(err)
			}

			// Write file
			fOut, err := os.Create(filename)
			if err != nil {
				log.Fatalln(err)
			}
			fOut.Write(data)
			fOut.Close()

			log.Println(filename, "write successful")
		},
	}
	cmdReceive.Flags().StringVarP(&Port, "port", "p", "", "serial port to connect to")
	cmdReceive.Flags().StringVarP(&Message, "message", "m", "", "message to initiate data transfer")
	cmdReceive.Flags().StringVarP(&Wait, "wait", "w", "", "message to wait before initiating data transfer")

	var Root = &cobra.Command{
		Use:   "go-xmodem",
		Short: "",
		Long:  ``,
		Run:   func(cmd *cobra.Command, args []string) {},
	}

	Root.AddCommand(cmdSend)
	Root.AddCommand(cmdReceive)
	Root.Execute()
}
