# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, January 24, 2020
.DEFAULT_GOAL = default
.PHONY = default debug release all run clean

MAKE = make
MAKEFLAGS =
DEBUG_MAKEFLAGS = --trace
RELEASE_MAKEFLAGS = -s

%:
	@:

__commond:
	$(MAKE) $(MAKEFLAGS) $(DEBUG_MAKEFLAGS) -C ./common debug

__common:
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./common release

__demosaicd: __commond
	$(MAKE) $(MAKEFLAGS) $(DEBUG_MAKEFLAGS) -C ./demosaic debug

__demosaic: __common
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./demosaic release

__histogramd: __commond
	$(MAKE) $(MAKEFLAGS) $(DEBUG_MAKEFLAGS) -C ./histogram debug

__histogram: __common
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./histogram release

__denoised: __commond
	$(MAKE) $(MAKEFLAGS) $(DEBUG_MAKEFLAGS) -C ./denoise debug

__denoise: __common
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./denoise release

debug: __demosaicd __histogramd __denoised

release: __demosaic __histogram __denoise

all: clean debug release

default: clean release

run: release
	@chmod +x ./run.sh
	@./run.sh $(filter-out $@,$(MAKECMDGOALS))

clean:
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./demosaic clean
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./histogram clean
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./denoise clean
	@$(MAKE) $(MAKEFLAGS) $(RELEASE_MAKEFLAGS) -C ./common clean
