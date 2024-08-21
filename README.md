# ebpf Summit 2024 Capture the Flag ⛳️🐝 

This is the repository containing all the code/files for the CTF for the upcoming (2024) eBPF summit.

## Building

Create the required ebpf headers `cd ebpf` & `./header.sh`


The `/emperium` folder contains the code for the **Emperium** system, this is the security system for the Empire's Tie Fighter systems. Access to this system would give the rebels the upper hand when it comes to X-Wing to Tie fighter combat. 

To build the stolen copy of the `emperium` system:

`cd emperium` & `go build`

## Create the CTF VM

`limactl start ctf.yaml`

## Accessing the VM

You will need at least two shells in order to access both the `emperium` system and the underlying OS in order to interact with the Kernel and eBPF programs.

### First shell

This will connect to the running system and start `emperium`.

`limactl shell ctf sudo /tmp/emperium`

### Second shell

The following line will connect through to the VM using SSH, it will also port forward from inside the VM (port 80) to port `8082` on your local machine. The reason for this is to allow the use of code-server to interact with the `eBPF` 🐝 code.

`ssh -F $HOME/.lima/ctf/ssh.config -L *:8082:0.0.0.0:80 lima-ctf`

To start `code-server` within the VM run the command `PASSWORD=password code-server --bind-addr=0.0.0.0`.

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
Security Lock 4>  [ ▮ ]
```

## Notes:

Clues:

- A hacker from the Zenith system in the employ of the Rebellion managed to get a communication out, whilst the quality was poor technicians managed to enhance enough of the audio to understand that the hacker may have "got it the wrong way around" and that the bpftool might be the best way to fix it.

- Our spies managed to extract one of the vital keycodes from the emperium key vaults, they unfortunately deleted the emperium map in the process. Whilst we now have this data `brRz3HVSVzC6RXrBC2Y7`, we're not sure if this will impact the running `emperium` system.

- A defector has provided most of the code that is needed in order to create a fake `emperium` mainframe, once up this will be able to answer the `emperium` system.
