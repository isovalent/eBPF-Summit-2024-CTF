package main

import (
	"fmt"

	"github.com/fatih/color"
)

const tie = `
             ._,.
           "..-..pf.
          -L   ..#'
        .+_L  ."]#
        ,'j' .+.j'                 -'.__..,.,p.
       _~ #..<..0.                 .J-.''..._f.
      .7..#_.. _f.                .....-..,'4'
      ;' ,#j.  T'      ..         ..J....,'.j'
     .' .."^.,-0.,,,,yMMMMM,.    ,-.J...+'.j@
    .'.'...' .yMMMMM0M@^='""g.. .'..J..".'.jH
    j' .'1'  q'^)@@#"^".'"='BNg_...,]_)'...0-
   .T ...I. j"    .'..+,_.'3#MMM0MggCBf....F.
   j/.+'.{..+       '^~'-^~~""""'"""?'"'''1'
   .... .y.}                  '.._-:'_...jf 
   g-.  .Lg'                 ..,..'-....,'.
  .'.   .Y^                  .....',].._f
  ......-f.                 .-,,.,.-:--&'
                            .'...'..'_J'
                            .~......'#'        Tie Fighter Manufacturing   
                            '..,,.,_]'           Security Systems
                            .L..'..''.
`

type securityLevel struct {
	lock1 bool
	lock2 bool
	lock3 bool
	lock4 bool
}

func InitSecurity() *securityLevel {
	fmt.Println("Security Status>", color.YellowString("Initialising..."))
	fmt.Println("Security Status>", color.GreenString("Enabled"))
	return &securityLevel{}
}

func (s *securityLevel) lock1Status() {
	if s.lock1 {
		fmt.Println("Security Lock 1> ", color.WhiteString("["), color.RedString("▮"), color.WhiteString("]"))
	} else {
		fmt.Println("Security Lock 1> ", color.WhiteString("["), color.GreenString("▮"), color.WhiteString("]"))
	}
}

func (s *securityLevel) lock2Status() {
	if s.lock2 {
		fmt.Println("Security Lock 2> ", color.WhiteString("["), color.RedString("▮"), color.WhiteString("]"))
	} else {
		fmt.Println("Security Lock 2> ", color.WhiteString("["), color.GreenString("▮"), color.WhiteString("]"))
	}
}

func (s *securityLevel) lock3Status() {
	if s.lock3 {
		fmt.Println("Security Lock 3> ", color.WhiteString("["), color.RedString("▮"), color.WhiteString("]"))
	} else {
		fmt.Println("Security Lock 3> ", color.WhiteString("["), color.GreenString("▮"), color.WhiteString("]"))
	}
}
func (s *securityLevel) lock4Status() {
	if s.lock4 {
		fmt.Println("Security Lock 4> ", color.WhiteString("["), color.RedString("▮"), color.WhiteString("]"))
	} else {
		fmt.Println("Security Lock 4> ", color.WhiteString("["), color.GreenString("▮"), color.WhiteString("]"))
	}
}

func (s *securityLevel) Status() {
	s.lock1Status()
	s.lock2Status()
	s.lock3Status()
	s.lock4Status()
}

func (s *securityLevel) Unlock1() {
	s.lock1 = true
	s.lock1Status()
}

func (s *securityLevel) Unlock2() {
	if !s.lock2 {
		s.lock2 = true
		s.lock2Status()
	}
}

func (s *securityLevel) Lock2() {
	if s.lock2 {
		s.lock2 = false
		s.lock2Status()
	}
}
