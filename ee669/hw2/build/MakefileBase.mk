.DEFAULT_GOAL = default

CC = gcc-8
CPP = g++-8
LD = $(CPP)
AR = gcc-ar-8

SRC_DIR = $(CURDIR)
OBJ_DIR = $(CURDIR)/../build/obj/$(PROJECT_NAME)
INC_DIR += $(CURDIR)/..
LIB_DIR = $(CURDIR)/../build/lib
APP_DIR = $(CURDIR)/../bin
3RD_PARTY_LIB_DIR = /usr/local/lib

CPPFLAGS = -std=c++17 $(addprefix -I, $(INC_DIR)) -c -fPIC -pthread
DEBUG_CPPFLAGS = -Wall -Wextra -g -D__DEBUG
RELEASE_CPPFLAGS = -O3

CCFLAGS = $(addprefix -I, $(INC_DIR)) -c -fPIC
DEBUG_CCFLAGS = -Wall -Wextra -g -D__DEBUG
RELEASE_CCFLAGS = -O3

LDFLAGS = -pthread -fPIC
DEBUG_LDFLAGS = -Wall
RELEASE_LDFLAGS =
APP_LDFLAGS = $(addprefix -I, $(INC_DIR)) -L$(LIB_DIR) $(addprefix -L, $(3RD_PARTY_LIB_DIR))
LIB_LDFLAGS = -shared

DEBUG_CPP_OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.d.o, $(notdir $(wildcard $(SRC_DIR)/*.cpp)))
RELEASE_CPP_OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(wildcard $(SRC_DIR)/*.cpp)))
DEBUG_CC_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.d.o, $(notdir $(wildcard $(SRC_DIR)/*.c)))
RELEASE_CC_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(wildcard $(SRC_DIR)/*.c)))

ifndef TARGET_NAME
	TARGET_NAME = $(PROJECT_NAME)
endif

ifdef 3RD_PARTY_LIBS
	DEBUG_LIBS_LDFLAGS += $(addprefix -l, $(3RD_PARTY_LIBS))
	RELEASE_LIBS_LDFLAGS += $(addprefix -l, $(3RD_PARTY_LIBS))
endif

ifeq ($(TYPE), LIB)
	DEBUG_TARGET = $(LIB_DIR)/lib$(TARGET_NAME)d.so
	RELEASE_TARGET = $(LIB_DIR)/lib$(TARGET_NAME).so
else
ifdef LIBS
	DEBUG_LIBS_LDFLAGS += $(addprefix -l, $(addsuffix d, $(LIBS)))
	RELEASE_LIBS_LDFLAGS += $(addprefix -l, $(LIBS))
endif
	DEBUG_CPPFLAGS += -D__PROJECT_NAME=\"$(PROJECT_NAME)d\"
	RELEASE_CPPFLAGS += -D__PROJECT_NAME=\"$(PROJECT_NAME)\"
	DEBUG_CCFLAGS += -D__PROJECT_NAME=\"$(PROJECT_NAME)d\"
	RELEASE_CCFLAGS += -D__PROJECT_NAME=\"$(PROJECT_NAME)\"
	DEBUG_TARGET = $(APP_DIR)/$(TARGET_NAME)d
	RELEASE_TARGET = $(APP_DIR)/$(TARGET_NAME)
endif

check:
ifndef PROJECT_NAME
	echo "Please specify PROJECT_NAME"
	@exit 1
endif

$(OBJ_DIR)/%.d.o: $(SRC_DIR)/%.cpp
	$(CPP) $(CPPFLAGS) $(DEBUG_CPPFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CPP) $(CPPFLAGS) $(RELEASE_CPPFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.d.o: $(SRC_DIR)/%.c
	$(CC) $(CCFLAGS) $(DEBUG_CCFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CCFLAGS) $(RELEASE_CCFLAGS) -o $@ -c $<

$(DEBUG_TARGET): $(DEBUG_CPP_OBJ) $(DEBUG_CC_OBJ)
ifeq ($(TYPE), LIB)
	$(LD) $(LDFLAGS) $(DEBUG_LDFLAGS) $(LIB_LDFLAGS) -o $@ $^ $(DEBUG_LIBS_LDFLAGS)
else
	$(LD) $(LDFLAGS) $(DEBUG_LDFLAGS) $(APP_LDFLAGS) -o $@ $^ $(DEBUG_LIBS_LDFLAGS)
endif

$(RELEASE_TARGET): $(RELEASE_CPP_OBJ) $(RELEASE_CC_OBJ)
ifeq ($(TYPE), LIB)
	$(LD) $(LDFLAGS) $(DEBUG_LDFLAGS) $(LIB_LDFLAGS) -o $@ $^ $(RELEASE_LIBS_LDFLAGS)
else
	$(LD) $(LDFLAGS) $(RELEASE_LDFLAGS) $(APP_LDFLAGS) -o $@ $^ $(RELEASE_LIBS_LDFLAGS)
endif

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(LIB_DIR):
ifeq ($(TYPE), LIB)
	@mkdir -p $(LIB_DIR)
endif

$(APP_DIR):
	@mkdir -p $(APP_DIR)

debug: check $(OBJ_DIR) $(LIB_DIR) $(APP_DIR) $(DEBUG_TARGET)

release: check $(OBJ_DIR) $(LIB_DIR) $(APP_DIR) $(RELEASE_TARGET)

all: debug release

default: release

clean:
	@rm -rf $(OBJ_DIR)
	@rm -f $(DEBUG_TARGET)
	@rm -f $(RELEASE_TARGET)
