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

__jpeg_6b_libd:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(LIB_DIR)/jpeg-6b-lib debug

__jpeg_6b_lib:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/jpeg-6b-lib release

__jpeg_libd: __commond __graphd __jpeg_6b_libd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(LIB_DIR)/jpeg-lib debug

__jpeg_lib: __common __graph __jpeg_6b_lib
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/jpeg-lib release

__dctd: __commond __graphd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(APP_DIR)/dct debug

__dct: __common __graph
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/dct release

__jpegd: __commond __graphd __jpeg_6b_libd __jpeg_libd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(APP_DIR)/jpeg debug

__jpeg: __common __graph __jpeg_6b_lib __jpeg_lib
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/jpeg release

__postprocessd: __commond __graphd __jpeg_6b_libd __jpeg_libd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(APP_DIR)/postprocess debug

__postprocess: __common __graph __jpeg_6b_lib __jpeg_lib
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/postprocess release

debug: __commond __graphd __jpeg_6b_libd __jpeg_libd __dctd __jpegd __postprocessd

release: __common __graph __jpeg_6b_lib __jpeg_lib __dct __jpeg __postprocess

all: debug release

default: release

clean:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/dct clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/jpeg clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(APP_DIR)/postprocess clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/jpeg-lib clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/common clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/graph clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(LIB_DIR)/jpeg-6b-lib clean
	@rm -rf $(CURDIR)/bin
	@rm -rf $(CURDIR)/build/lib
	@rm -rf $(CURDIR)/build/obj
