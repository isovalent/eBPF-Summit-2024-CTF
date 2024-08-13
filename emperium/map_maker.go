package main

import (
	"fmt"
	"strings"
	"time"

	"math/rand"

	"github.com/fatih/color"
)

const letterBytes = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
const (
	letterIdxBits = 6                    // 6 bits to represent a letter index
	letterIdxMask = 1<<letterIdxBits - 1 // All 1-bits, as many as letterIdxBits
	letterIdxMax  = 63 / letterIdxBits   // # of letter indices fitting in 63 bits
)

var src = rand.NewSource(time.Now().UnixNano())

func create_maps(count int) (name, contents []string) {
	fmt.Println("Data system>", color.YellowString("Initialising..."))
	// Lets create many maps
	for i := 0; i < count; i++ {
		name = append(name, fmt.Sprintf("empire_%s", RandStringBytesMaskImprSrcSB(3)))
		contents = append(contents, RandStringBytesMaskImprSrcSB(20))
	}
	return
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
