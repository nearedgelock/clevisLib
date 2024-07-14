# clevisLib
Wrapper to José providing composite clevis functions

## Purpose
This library provides mid-level composite functions (encryption. decryption, key generation, valadition, etc.) in an abstract way to build a clevis compatible client. See [Clevis](http://github.com/latchset/clevis).

The author needed to create both a compiled command line tools as well as a WebAssembly variant so having a common mid-level library was deemed reasonable. 

A WASM binding layer was also added.

## Using
In fact, this library is simply a submodule to other repositories. As such building is not necessary here. However, for testing, debugging and development purpose everything required to build a basic test bed is provided.

### Dependencies
The library requires a number of external module and are directly included herein via git's submodule OR via vcpkg (which itself is included as a submodule).

### Compiling the local test program
```bash
make bootstrap    <-- Only on first time
make clean        <-- Not necessary when the first time
make configure    <-- First time, or if you change any of the cmake related files.
make build
```
### Cross-compiling with emscripten
This is primarily to validate compilation. A JS file is generated and could be imported into your test code.

The first step is to setup the emscripten cross-compiler. This is required only once (or after an update of the cross-compiler). Use
```bash
cd emsdk
./emsdk install latest
./emsdk activate latest
```

Next, setup your environment (mostly the PATH variable) using the emscripten provided script. Do this each time you open a terminal.
```bash
source ./emsdk_env.sh
```

Lasty, to compile clevisLib for inclusion into a WASM project, use
```bash
make bootstrap        <-- Only on first time
make clean            <-- Not necessary when the first time
make configureWASM    <-- First time, or if you change any of the cmake related files.
make build
```