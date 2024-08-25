# diagnostic-tui

simple TUI to see some system diagnostics

## Installation

```bash
git clone https://github.com/BenSimmers/diagnostic-tui.git
```

- Use the Makefile to build and run the program

```bash
# build the program
make
```


```bash
# run the program
./bin/sys_monitor

# or 
arch -arm64 ./bin/sys_monitor   # Run as ARM64
# or (if rosetta is installed)
arch -x86_64 ./bin/sys_monitor  # Run as x86_64 (if you have Rosetta installed)
```

```bash
# clean the program
make clean
```


