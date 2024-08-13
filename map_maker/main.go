package main

import (
	"fmt"
	log "log/slog"
	"math/rand"
	"os"
	"strings"
	"time"
)

const file = `
package main

type mapConfig struct {
	name     string
	contents string
	position int
}

var generatedMaps = [...]mapConfig {
%s
}
`

const letterBytes = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
const (
	letterIdxBits = 6                    // 6 bits to represent a letter index
	letterIdxMask = 1<<letterIdxBits - 1 // All 1-bits, as many as letterIdxBits
	letterIdxMax  = 63 / letterIdxBits   // # of letter indices fitting in 63 bits
)

var src = rand.NewSource(time.Now().UnixNano())

func main() {
	log.Info("Creating Map file")
	// Lets create many maps
	var sb strings.Builder
	for i := 0; i < 50000; i++ {
		name := fmt.Sprintf("empire_%s", RandStringBytesMaskImprSrcSB(3))
		contents := []byte(RandStringBytesMaskImprSrcSB(20))
		sb.WriteString(fmt.Sprintf("{\"%s\", \"%s\", 0},\n", name, contents))
	}
	f, err := os.Create("../loader/maps.go")
	if err != nil {
		panic(err)
	}
	f.WriteString(fmt.Sprintf(file, sb.String()))
	defer f.Close()

}

func RandStringBytesMaskImprSrcSB(n int) string {
	sb := strings.Builder{}
	sb.Grow(n)
	// A src.Int63() generates 63 random bits, enough for letterIdxMax characters!
	for i, cache, remain := n-1, src.Int63(), letterIdxMax; i >= 0; {
		if remain == 0 {
			cache, remain = src.Int63(), letterIdxMax
		}
		if idx := int(cache & letterIdxMask); idx < len(letterBytes) {
			sb.WriteByte(letterBytes[idx])
			i--
		}
		cache >>= letterIdxBits
		remain--
	}

	return sb.String()
}
