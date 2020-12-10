.DEFAULT_GOAL = default
.PHONY = debug release all default clean

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

__x264:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/x264 -f x264.mk release

__encoderd: __commond __graphd __x264
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(APP_DIR)/encoder debug

__encoder: __common __graph __x264
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/encoder release

debug: __commond __graphd __x264 __encoderd

release: __common __graph __x264 __encoder

all: debug release

default: release

clean:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/encoder clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/common clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/graph clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/x264 -f x264.mk clean
	@rm -rf $(CURDIR)/bin
	@rm -rf $(CURDIR)/build/lib
	@rm -rf $(CURDIR)/build/obj
