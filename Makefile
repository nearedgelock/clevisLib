ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
$(eval $(ARGS):;@:)
makefileDir := $(dir $(firstword $(MAKEFILE_LIST)))

VCPKG_DEFAULT_TRIPLET := emscripten
EMCMAKE := $(makefileDir)emsdk/upstream/emscripten/emcmake
NODE := /home/crobitaille/.nvm/versions/node/v16.13.0/bin/node
CMAKE := /usr/bin/cmake

default: build

.PHONY: bootstrap
bootstrap:
	git submodule update --init vcpkg && vcpkg/bootstrap-vcpkg.sh --disableMetrics

.PHONY: clean
clean:
	rm -rf build
	rm -rf vcpkg/buildtrees

.PHONY: configureDebug
configureDebug:
	@$(CMAKE) cmake -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -D CMAKE_BUILD_TYPE=Debug -DNATIVE=ON -DBUILD_EXECUTABLE=On

.PHONY: configure
configure:
	@$(CMAKE) cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -D CMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBUILD_EXECUTABLE=On

.PHONY: configureWASMDebug
configureWASMDebug:
	@$(EMCMAKE) cmake -S . -B build -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXE_LINKER_FLAGS=\"-static\"-D CMAKE_BUILD_TYPE=Debug -DWEB_TARGET=1 -DBUILD_EXECUTABLE=Off

.PHONY: configureWASM
configureWASM:
	@$(EMCMAKE) cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXE_LINKER_FLAGS=\"-static\" -D CMAKE_BUILD_TYPE=Release -DWEB_TARGET=1 -DBUILD_EXECUTABLE=Off

.PHONY: build
build:
	@$(CMAKE) --build build

