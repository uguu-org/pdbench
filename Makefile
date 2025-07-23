# Toplevel Makefile for Playdate Benchmark project.
#
# This Makefile only needs common tools that are available through Cygwin.
#
# See source/Makefile for additional tools needed to build binaries.
#
# To build release packages:
#
#   make clean && make -j

ifeq ($(PLAYDATE_SDK_PATH),)
$(error need to set PLAYDATE_SDK_PATH environment)
endif

PACKAGE_NAME = pd_bench
SIM_SOURCE = sim_build_source
DEVICE_SOURCE = device_build_source

all: $(PACKAGE_NAME).zip $(PACKAGE_NAME)_windows.zip

# Build rules for package with extra Windows DLL, and without any lua code.
$(PACKAGE_NAME)_windows.zip: $(PACKAGE_NAME)_windows.pdx
	rm -f $@
	zip -9 -r $@ $<

$(PACKAGE_NAME)_windows.pdx: windows_source
	"$(PLAYDATE_SDK_PATH)/bin/pdc" -s $(SIM_SOURCE) $@

windows_source: device_source source/sim_build/pdex.dll
	mkdir -p $(SIM_SOURCE)
	cp -R `find $(DEVICE_SOURCE) -mindepth 1 -maxdepth 1 | grep -vF .lua` $(SIM_SOURCE)/
	cp source/sim_build/pdex.dll $(SIM_SOURCE)/

# Build rules for device-only package.
$(PACKAGE_NAME).zip: $(PACKAGE_NAME).pdx
	rm -f $@
	zip -9 -r $@ $<

$(PACKAGE_NAME).pdx: device_source
	"$(PLAYDATE_SDK_PATH)/bin/pdc" -s $(DEVICE_SOURCE) $@

device_source: source/device_build/pdex.elf source/build/launcher source/pdxinfo source/main.lua
	mkdir -p $(DEVICE_SOURCE)
	cp -R $^ $(DEVICE_SOURCE)/

# Build dependencies for package input files.
source/sim_build/pdex.dll: | build_source

source/device_build/pdex.elf: | build_source

source/build/launcher: | build_source

build_source:
	$(MAKE) -C source


# Maintenance rules.
clean:
	$(MAKE) -C source clean
	rm -rf $(SIM_SOURCE) $(DEVICE_SOURCE)
	rm -rf $(PACKAGE_NAME).{pdx,zip} $(PACKAGE_NAME)_windows.{pdx,zip}
