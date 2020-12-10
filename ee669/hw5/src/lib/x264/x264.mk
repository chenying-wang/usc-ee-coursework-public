include config.mak

.DEFAULT_GOAL = default
.PHONY = debug release all clean default

MAKE = make
MAKE_FLAGS = -k
RELEASE_MAKE_FLAGS = -s

CONFIGURE = $(CURDIR)/configure
CONFIGURE_FLAGS = --enable-pic --enable-shared --disable-cli --bit-depth=8 --chroma-format=420 --extra-cflags=-DEE669_X264_ME
CONFIGURE_RELEASE_FLAGS = --enable-lto --extra-cflags=-DEE669_X264_NO_LOG

TARGET = lib-shared
LIB_DIR = $(CURDIR)/../../../build/lib
LIB_NAME = x264

config.mak:
	@$(CONFIGURE) $(CONFIGURE_FLAGS) $(CONFIGURE_RELEASE_FLAGS)

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

release: config.mak $(LIB_DIR)
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) $(TARGET)
	@mv -u $(SONAME) $(LIB_DIR)/$(SONAME)
	@ln -sf $(SONAME) $(LIB_DIR)/lib$(LIB_NAME).so

clean:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) distclean
	@rm -f $(LIB_DIR)/$(SONAME)
	@rm -f $(LIB_DIR)/lib$(LIB_NAME).so

default: release
