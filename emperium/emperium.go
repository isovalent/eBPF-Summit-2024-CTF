package main

import (
	"fmt"
	log "log/slog"
	"net"
	"os"
	"strings"
	"time"

	"github.com/cilium/ebpf"
	"github.com/fatih/color"
)

// pass map pointers once all the maps have been created as finding existing maps seems impossible
func (s *securityLevel) keyWatch(watchedMaps [4]*ebpf.Map) {

	_, first := os.LookupEnv("SKIPFIRST")
	_, second := os.LookupEnv("SKIPSECOND")
	_, third := os.LookupEnv("SKIPTHIRD")
	_, fourth := os.LookupEnv("SKIPFORTH")

	if !first {
		s.firstLock(watchedMaps[0])
	}
	if !second {
		s.secondLock(watchedMaps[1])
	}
	if !third {
		s.firstLock(watchedMaps[2])
	}
	if !fourth {
		s.firstLock(watchedMaps[3])
	}

}

func (s *securityLevel) firstLock(m *ebpf.Map) error {
	var value [20]byte
	err := m.Lookup(uint8(1), &value)
	if err != nil {
		log.Error(fmt.Sprintf("This shouldn't occur [%v]", err))
	}
	strValue := string(value[:])
	correctValue := Reverse(strValue)
	// Horrific string manipulation as we can't get JUST the name of the map
	a := strings.Split(m.String(), "(")
	b := strings.Split(a[1], ")")
	fmt.Println("Data system>", color.RedString(fmt.Sprintf("Map [%s] is corrupt", b[0])))

	for {
		err := m.Lookup(uint8(1), &value)
		if err != nil {
			log.Error(fmt.Sprintf("This shouldn't occur [%v]", err))
		}
		time.Sleep(time.Second * 5) // We check this map
		strValue := string(value[:])
		if strValue == correctValue {
			s.Unlock1()
			break // pop out the loop
		}
	}
	return nil
}

func (s *securityLevel) secondLock(m *ebpf.Map) error {
	fmt.Println("Connect>", color.YellowString("Empire local Mainframe"))
	conn, err := net.DialUDP("udp", nil, &net.UDPAddr{
		Port: 9000,
	})
	if err != nil {
		panic(err)
	}
	defer conn.Close()
	response := make([]byte, 3) // Get map identifier

	// go func() {
	// 	conn, err := net.ListenPacket("udp", ":9000")
	// 	if err != nil {
	// 		panic(err)
	// 	}
	// 	defer conn.Close()
	// 	buf := make([]byte, 1024)
	// 	for {
	// 		_, addr, err := conn.ReadFrom(buf)
	// 		if err != nil {
	// 			panic(err)
	// 		}
	// 		log.Info(fmt.Sprintf("Received  %s  from  %s", string(buf), addr))
	// 		conn.WriteTo([]byte("Hello from UDP server"), addr)
	// 	}
	// }()

	// Attempt to send data (4 bytes) to the remote address
	for {

		_, err = conn.Write([]byte("MAP\n"))
		if err != nil {
			log.Error(fmt.Sprintf("%v", err))
		}
		conn.SetReadDeadline(time.Now().Add(time.Second))

		_, _, err = conn.ReadFromUDP(response)
		if err != nil {
			fmt.Println("Connect>", color.RedString("Mainframe connection failure"))
			// log.Error(fmt.Sprintf("%v", err))
		} else {
			if string(response) != "DAN" {
				fmt.Println("Connect>", color.RedString("Mainframe sent incorrect response"))
			}
		}
		time.Sleep(time.Second * 10)
	}
}
