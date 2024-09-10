# eBPF Summit 2024 Capture the Flag ⛳️🐝 

Welcome to the eBPF 2024 Summit CTF, this year the task will involve a number of challenges around various eBPF technologies. Some of the challenges will require use of CLI tools such as `bpftool` and some will require fixing or completing some partial code in order to complete the challenge.

## Getting help and solving the CTF

Please register at the [eBPF slack](https://slack.cilium.io) and join the `#ebpf-summit` & `#ebpf-summit-ctf-spoilers` channels! Here you will find help and clues, and also a place to submit your answers once you've completed the CTF!.

## Getting the CTF up and running !

The CTF makes use of the [lima](https://github.com/lima-vm/lima) project to simplify the process of creating a virtual machine that will run the CTF. The virtual machine once created will contain:

- CTF program
- code-server (VS-CODE but accessed through a browser)
- vim
- a `~/ebpf` directory that is filled with the sample code for the challenges

## Create the CTF VM
You will need to take the `ctf.yaml` in this repository and start it with `limactl`, this will build and start the virtual machine!

`limactl start ctf.yaml`

## Accessing the VM

You will need at least two shells in order to access both the `emperium` system and the underlying OS in order to interact with the Kernel and eBPF programs.

### First shell

This will connect to the running system and start `emperium`.

`limactl shell ctf sudo /tmp/emperium`

```

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
Data system> Initialising...
Data system> Ready
Security Status> Enabled
Security Lock 1>  [ ▮ ]
Security Lock 2>  [ ▮ ]
Security Lock 3>  [ ▮ ]
```

### Second shell

The following line will connect through to the VM using SSH, it will also port forward from inside the VM (port 80) to port `8082` on your local machine. The reason for this is to allow the use of code-server to interact with the `eBPF` 🐝 code.

`ssh -F $HOME/.lima/ctf/ssh.config -L "*:8082:0.0.0.0:80" lima-ctf`

To start `code-server` within the VM run the command: 

`PASSWORD=password code-server --bind-addr=0.0.0.0 > /tmp/code.log &`.

**Additionally**: You may find it makes things a little easier to install some plugins to make things easier, such as the `clangd` extension or the `golang` extension.

## Setting up the eBPF code

All of the code lives within `~/eBPF` and is automatically created when you start the first shell, so if this is missing ensure you're following the instructions.

### Code-Server or Vim

To make life as easy as possible we've packaged both Vim and Code-server for you to create and modify Go and eBPF code. To connect to code-server open a browser and point it to the IP address of the host where the `lima` VM is running (not the CTF VM IP, the IP of the hosting machine itself). Connecting to port `8082` will open code-server that will allow you to work inside the CTF VM. The CTF source code is in your home directory!

### Compiling the code

In each of the folders in `~/eBPF` is some half completed eBPF 🐝 programs. Most of the programs will require running `go generate` in order to compile the C/eBPF source code first before you can run the main go binary. Additionally to run the go programs themselves the easiest approach is to either `go build` and run the resulting binary with `sudo` or the do `go run -exec sudo .`.

## The challenge !

They are _everywhere_. This is a true infestation. Way worse than the rockmites you had to fight off to defend your giant bees, back in your days as a beekeeper. Worse, even, than the herd of mynocks you found in that asteroid field, when ambushing an Imperial transporter (which turned up to carry no sensitive information, but never mind, they freighted chocolates, and they're delicious). This time, they just won't stop coming. And they're big things, flying fast, with a terrible roar.

These are TIE Fighters from the Empire. And they're after you.

Jephen'Tsa is your name, and you've made life tough for the Empire. But this time, Imperials are the ones harassing your team. If they can't pin down the exact location of your Rebel base, they have a gross idea of the area where you're hiding, courtesy of your sworn enemy, the traitor Bajeroff Lake. Armadas of TIEs have been patrolling in the whole constellation, and every mission, every outing has been a pain. Sure, the Rebel Alliance has some of the best pilots in the galaxy, but for each fighter they strike down, five new ships come out of the factory. Your own vessel, the _Yellow Stripe_, is docked for repairs. This is untenable. If you could just stop enemy fighters from coming back...

As it turns out, an opportunity came up this morning, thanks to the hard work of your friend and ally, Blue Hex. Her utility droid, IP-V6, just brought you to the control room, where Blue Hex explains how her spy network is getting close to disable the whole TIE fighter production lines, no less! She has established a hidden connection to the mainframe in the manufacturing facility, but disabling it requires accessing the core controls of the system, protected by **four** security locks. Oh wait, an alarm erupts! Some TIEs are nearby! Did they find your hideout at last? Blue Hex stops her briefing and jumps into her ship, imitated by the other pilots.

But your own spacecraft is not ready, and for this time you're grounded. You should as well look into these security locks, to disable the factory. That would be quite an achievement... One more to add to your name? Sit down and relax. The Force is with you. You can do it. Let's start by sending IP-V6 fetch a few more of these delicious chocolates...


### Part 1 - unscramble the message 
A hacker from the Zenith system in the employ of the Rebellion managed to get a communication out, whilst the quality was poor technicians managed to enhance enough of the audio to understand that the hacker may have "got it the wrong way around" and that the bpftool might be the best way to fix it.

<details>
<summary>First Clue</summary>
  
  `bpftool` can be used to list all of the maps
<details>
<summary>Second Clue</summary>
  
  `bpftool` can dump the contents of a specific map identified by name or ID
<details>
<summary>Final Clue</summary>
  
  `bpftool` can update the contents of the specific key/value within a map.
</details>
</details>
</details>

### Part 2 - complete the keycodes
Our spies managed to extract one of the vital keycodes from the emperium key vaults, they unfortunately deleted the emperium map in the process. Whilst we now have this data `brRz3HVSVzC6RXrBC2Y7`, we're not sure if this will impact the running `emperium` system. 📂`eBPF/map` (or perhaps this can be done with `bpftool` 🤔)

<details>
<summary>First Clue</summary>
  
  Partially completed code is one way to help you achieve this, you'll need to look at an existing map to understand the key/values
<details>
<summary>Second Clue</summary>
  
  An `eBPF` map will only exist as long as a program or file has a reference too it, otherwise it will be garbage collected.
</details>
</details>

### Part 3 - get authenticated  
 An archive taken from a stolen ship has revealed the third security lock is broken due to the authentication being pushed to the wrong port. One of the engineers has managed to put something together, but keeps muttering about "Endianness" and returning traffic. 📂`eBPF/redirect/`

<details>
<summary>First Clue</summary>
  
  Is something generating network traffic on the `lo` interface? 
<details>
<summary>Second Clue</summary>
  
  In most cases, numbers are carried in network packets in [big-endian](https://en.wikipedia.org/wiki/Endianness#/media/File:32bit-Endianess.svg) byte order, which may differ from the host byte order on a particular machine. So often you may need to convert between host byte order and network byte order. There are bpf helper functions that will allow you to convert between the two.
  
<details>
<summary>Third Clue</summary>
  
  Changing a destination port will effectively change where traffic is being sent to, although it may confuse the networking stack to suddenly receive a reply to a port that it wasn't expecting...
</details>
</details>
</details>
  
### Part 4 - access the mainframe 
 A defector has provided most of the code that is needed in order to create a fake `emperium` mainframe, once up this will be able to **"acknowledge"** the `emperium` system. The specialist that wrote most of this was reassigned after breaking his keyboard about a "verifier"? 📂`eBPF/response/`

<details>
<summary>First Clue</summary>
  
You'll need to modify the partially-complete code so that it passes the eBPF Verifier
  
<details>
<summary>Second Clue</summary>
  
There are BPF helper functions for reading bytes out of the kernel's network packet data structure (a.k.a. `skb`) as well as for storing bytes in it. 
</details>
</details>

## Additional

You can also run the `/emperium` program locally (with root privileges) on a Linux machine if you don't want to use lima. It will attempt to write the source code to a `eBPF` folder so ensure one doesn't exist in the directory you run the `/emperium` program.
