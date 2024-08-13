package main

import (
	"fmt"
	log "log/slog"
	"os"
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

	// Create our templatae map spec
	mapSpec := ebpf.MapSpec{
		Type:       ebpf.Hash,
		KeySize:    1, // 4 bytes for u32
		ValueSize:  20,
		MaxEntries: 1, // We'll have 5 maps inside this map
	}
	var watchedMaps [4]*ebpf.Map  // Hold the four maps that we care about
	var unWatchedMaps []*ebpf.Map // Hold references to maps we don't

	name, contents := create_maps(10000) // Generate unique names/contents for the maps
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
		// if generatedMaps[i].position != 0 {
		// 	watchedMaps[(generatedMaps[i].position - 1)] = m
		// } else {
		unWatchedMaps = append(unWatchedMaps, m) // This is so we keep references to all the maps (stops the )
		// }
	}

	fmt.Println("Data system>", color.GreenString("Ready"))

	_, garbage := os.LookupEnv("SKIPGARBAGE")
	if !garbage {
		// Not sure why this is needed at this time
		go func() {
			//give the maps a little prod, to stop them being cleaned up
			for i := range unWatchedMaps {
				time.Sleep(time.Minute)
				_ = unWatchedMaps[i].KeySize()
			}
		}()
	}
	// Start the Tie Fighter security systems
	s := InitSecurity()
	s.Status()
	go func() {
		s.keyWatch(watchedMaps)
	}()

	time.Sleep(time.Minute * 5)
}
