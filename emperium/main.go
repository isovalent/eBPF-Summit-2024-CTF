package main

import (
	"fmt"
	log "log/slog"
	"time"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/rlimit"
	"github.com/fatih/color"
)

func main() {
	// Allow the current process to lock memory for eBPF resources.
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Error("rlimit")
		panic(err)
	}

	fmt.Print(tie) // Print the main logo
	s := InitSecurity()

	// Create our templatae map spec
	mapSpec := ebpf.MapSpec{
		Type:       ebpf.Hash,
		KeySize:    1, // 4 bytes for u32
		ValueSize:  20,
		MaxEntries: 1, // We'll have 5 maps inside this map
	}
	var watchedMaps [4]*ebpf.Map // Hold the four maps that we care about

	name, contents := create_maps(1000) // Generate unique names/contents for the maps
	//var validMaps = map[int]bool
	// Lets create many maps
	for i := range name {
		mapSpec.Name = name[i]
		mapSpec.Contents = []ebpf.MapKV{
			{Key: uint8(1), Value: []byte(contents[i])},
		}
		m, err := ebpf.NewMap(&mapSpec)
		if err != nil {
			log.Error("map create fail ")
			panic(err)
		}
		defer m.Close()
		if i < 4 { // Just grab the first four maps for now // TODO:
			watchedMaps[i] = m
		}
	}

	fmt.Println("Data system>", color.GreenString("Ready"))
	// Start the Tie Fighter security systems
	s.Status()
	go func() {
		s.keyWatch(watchedMaps)
	}()

	time.Sleep(time.Minute * 30)
}
