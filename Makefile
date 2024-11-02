ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
$(eval $(ARGS):;@:)
makefileDir := $(dir $(firstword $(MAKEFILE_LIST)))

EMCMAKE := $(makefileDir)emsdk/upstream/emscripten/emcmake
CMAKE := /usr/bin/cmake

default: build

#
# Clean up
#	
.PHONY: cleanAll
cleanAll:
	rm -rf build

.PHONY: cleanOurs
cleanOurs:
	@find build -mindepth 1 -maxdepth 1 ! -name '_deps' -exec rm -rf {} +

.PHONY: clean
clean: cleanOurs	

.PHONY: configureDebugLib
configureDebugLib:
	@$(CMAKE) -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON -D CMAKE_BUILD_TYPE=Debug -DNATIVE=ON -DBUILD_EXECUTABLE=OFF -DCMAKE_INSTALL_PREFIX=build/install

.PHONY: configureDebug
configureDebug:
	@$(CMAKE) -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON -D CMAKE_BUILD_TYPE=Debug -DNATIVE=ON -DBUILD_EXECUTABLE=ON

.PHONY: configure
configure:
	@$(CMAKE) -S . -B build -D CMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBUILD_EXECUTABLE=ON

.PHONY: configureWASMDebug
configureWASMDebug:
	@$(EMCMAKE) cmake -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_EXE_LINKER_FLAGS=\"-static\"-D CMAKE_BUILD_TYPE=Debug -DNATIVE=OFF -DWEB_TARGET=1

.PHONY: configureWASM
configureWASM:
	@$(EMCMAKE) cmake -S . -B build -DCMAKE_EXE_LINKER_FLAGS=\"-static\" -D CMAKE_BUILD_TYPE=Release -DNATIVE=OFF -DWEB_TARGET=1

.PHONY: build
build:
	@$(CMAKE) --build build

# This currently create a package and cmake when not building an execut5able and not targetting the emscripten cross-compiler
# The goal is to allow the use of this as a library to other cmake projects
.PHONY: install
install:
	@$(CMAKE) --install build

