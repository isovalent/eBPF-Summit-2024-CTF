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
		s.thirdLock(watchedMaps[2])
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
	fmt.Println("Data system>", color.RedString("Map ["), color.WhiteString(string(b[0])), color.RedString("] is corrupt!"))

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
func (s *securityLevel) secondLock(ma *ebpf.Map) error {
	var value [20]byte
	var m *ebpf.Map
	mapName := fmt.Sprintf("empire_%s", RandStringBytesMaskImprSrcSB(3))
	for {
		found, correct := false, false
		for id := ebpf.MapID(0); ; {
			var err error
			id, err = ebpf.MapGetNextID(ebpf.MapID(id))
			if err != nil {
				break
			}
			m, err = ebpf.NewMapFromID(id)
			if err != nil {
				panic(err)
			}
			info, err := m.Info()
			if err != nil {
				panic(err)
			}

			if info.Name == mapName {
				found = true

				err := m.Lookup(uint8(1), &value)

				if err != nil {
					log.Info(fmt.Sprintf("%v %d %d %d,", err, m.ValueSize(), m.FD(), id))
					fmt.Println("Data system>", color.YellowString("Map ["), color.WhiteString(mapName), color.YellowString("] has no data!"))
					continue
				}
				strValue := string(value[:])
				if strValue == "brRz3HVSVzC6RXrBC2Y7" {
					correct = true
					m.Close()
					break // pop out the loop
				} else {
					continue
				}
			}
			m.Close()
		}

		// After checking all maps, see if the map was found
		if found {
			if correct {
				s.Unlock2()
			} else {
				fmt.Println("Data system>", color.YellowString("Map ["), color.WhiteString(mapName), color.YellowString("] has incorrect data!"))
			}
		} else {
			fmt.Println("Data system>", color.RedString("Map ["), color.WhiteString(mapName), color.RedString("] is missing!"))
			s.Lock2()
		}
		time.Sleep(time.Second * 5)

	}
}

func (s *securityLevel) thirdLock(m *ebpf.Map) error {
	fmt.Println("Connect>", color.YellowString("Empire local Mainframe"))
	conn, err := net.DialUDP("udp", nil, &net.UDPAddr{
		Port: 9000,
	})
	if err != nil {
		panic(err)
	}
	defer conn.Close()
	response := make([]byte, 3) // Get map identifier

	// Attempt to send data (3 bytes) to the remote address
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
			} else {
				fmt.Println("Connect>", color.GreenString("Mainframe acknowledged response"))
				s.Unlock2()
				break // pop out the loop
			}
		}
		time.Sleep(time.Second * 10)
	}
	return nil
}
