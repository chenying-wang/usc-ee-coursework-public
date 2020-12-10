.DEFAULT_GOAL = default
.PHONY = default debug release all run clean

MAKE = make
MAKE_FLAGS = -k
DEBUG_MAKE_FLAGS = --trace
RELEASE_MAKE_FLAGS = -s

__commond:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/common debug

__common:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/common release

__third_partyd:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/third_party debug

__third_party:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/third_party release

__encoderd: __commond __third_partyd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/encoder debug

__encoder: __common __third_party
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/encoder release

debug: __commond __encoderd

release: __common __encoder

all: clean debug release

default: clean release

clean:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/encoder clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/common clean
	@rm -rf $(CURDIR)/bin
	@rm -rf $(CURDIR)/build/lib
	@rm -rf $(CURDIR)/build/obj
