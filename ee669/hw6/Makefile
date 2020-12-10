.DEFAULT_GOAL = default
.PHONY = default debug release all run clean

MAKE = make
MAKE_FLAGS = -k
DEBUG_MAKE_FLAGS = --trace
RELEASE_MAKE_FLAGS = -s

APP_DIR = $(CURDIR)/src/app
LIB_DIR = $(CURDIR)/src/lib

__commond:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(LIB_DIR)/common debug

__common:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/common release

__graphd: __commond
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(LIB_DIR)/graph debug

__graph: __common
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/graph release

__statd: __commond __graphd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(APP_DIR)/stat debug

__stat: __common __graph
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/stat release

debug: __commond __graphd __statd

release: __common __graph __stat

all: debug release

default: release

clean:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/stat clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/common clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/graph clean
	@rm -rf $(CURDIR)/bin
	@rm -rf $(CURDIR)/build/lib
	@rm -rf $(CURDIR)/build/obj
