ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
$(eval $(ARGS):;@:)
makefileDir := $(dir $(firstword $(MAKEFILE_LIST)))

EMCMAKE := $(makefileDir)emsdk/upstream/emscripten/emcmake
CMAKE := /usr/bin/cmake

default: build

.PHONY: bootstrap
bootstrap:
	git submodule update --init vcpkg && vcpkg/bootstrap-vcpkg.sh --disableMetrics

.PHONY: clean
clean:
	rm -rf build

.PHONY: configureDebug
configureDebug:
	@$(CMAKE) -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON -D CMAKE_BUILD_TYPE=Debug -DNATIVE=ON -DBUILD_EXECUTABLE=Off -DCMAKE_INSTALL_PREFIX=build/install

.PHONY: configure
configure:
	@$(CMAKE) -S . -B build -D CMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBUILD_EXECUTABLE=On -DCMAKE_INSTALL_PREFIX=build/install

.PHONY: configureWASMDebug
configureWASMDebug:
	@$(EMCMAKE) cmake -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_EXE_LINKER_FLAGS=\"-static\"-D CMAKE_BUILD_TYPE=Debug -DWEB_TARGET=1 -DBUILD_EXECUTABLE=Off

.PHONY: configureWASM
configureWASM:
	@$(EMCMAKE) cmake -S . -B build -DCMAKE_EXE_LINKER_FLAGS=\"-static\" -D CMAKE_BUILD_TYPE=Release -DWEB_TARGET=1 -DBUILD_EXECUTABLE=Off

.PHONY: build
build:
	@$(CMAKE) --build build

.PHONY: install
install:
	@$(CMAKE) --install build

