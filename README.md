# clevisLib
Wrapper to José providing composite clevis functions

## Purpose
This library provides mid-level composite functions (encryption. decryption, key generation, valadition, etc.) in an abstract way to build a clevis compatible client. See [Clevis](http://github.com/latchset/clevis).

The author needed to create both a compiled command line tools as well as a WebAssembly variant so having a common mid-level library was deemed reasonable. 

A WASM binding layer was also added.

## Using
In fact, this library is simply a submodule to other repositories. As such building is not necessary here. However, for testing, debugging and development purpose everthing required to build a basic test bed is provided.

### Dependencies
The library requires a number of external module and are directly included herein via git's submodule OR via vcpkg (which itself in included as a submodule).

### Compiling the local test program
```bash
make bootstrap
make configure
make build
```
### Cross-compiling wiht emscripten
This is primarily to validate compilation. A JS file is generated and could be imported into your test code.

The first step is to setup the emscripten cross-compile. This is requried only once (or after an update of the cross-compiler). Use
```bash
cd emsdk
./emsdk install latest
./emsdk activate latest
```

Next, setup youer environment (mostly the PATh variable) using the emscripten provided script. Do this each time you open a terminal.
```bash
source ./emsdk_env.sh
```

Lasty, to compile clevisLib, use
```bash
make bootstrap
make configureWASM
make build
```