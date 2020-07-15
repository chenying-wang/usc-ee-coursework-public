.DEFAULT_GOAL = default
.PHONY = default debug release all run clean

MAKE = make
MAKE_FLAGS = -k
DEBUG_MAKE_FLAGS = --trace
RELEASE_MAKE_FLAGS = -s

%:
	@:

__serverd:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/server debug

__server:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/server release

__clientd:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/client debug

__client:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/client release

debug: __serverd __clientd

release: __server __client

all: clean debug release

default: clean release

clean:
	@rm -rf $(CURDIR)/bin
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/server clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/client clean
	@rm -rf $(CURDIR)/build/lib
	@rm -rf $(CURDIR)/build/obj
