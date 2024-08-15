package main

import (
	log "log/slog"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/rlimit"
)

func main() {
	// Allow the current process to lock memory for eBPF resources.
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Error("rlimit")
		panic(err)
	}
	// Create our templatae map spec
	mapSpec := ebpf.MapSpec{
		Type:       ebpf.Hash,
		KeySize:    1, // 4 bytes for u32
		ValueSize:  20,
		MaxEntries: 1, // We'll have 1 entry
	}

	// Lets create a map
	mapSpec.Name = ""
	mapSpec.Contents = []ebpf.MapKV{
		{Key: uint8(1), Value: []byte("")},
	}
	m, err := ebpf.NewMap(&mapSpec)
	if err != nil {
		log.Error("map create fail ")
		panic(err)
	}
	defer m.Close()
	//time.Sleep(time.Minute)
}
