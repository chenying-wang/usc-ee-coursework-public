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

__healthcenterd: __commond
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/healthcenter debug

__healthcenter: __common
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/healthcenter release

__hospitald: __commond
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/hospital debug

__hospital: __common
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/hospital release

__studentd: __commond
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/student debug

__student: __common
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/student release

debug: __commond __healthcenterd __hospitald __studentd

release: __common __healthcenter __hospital __student

all: clean debug release

default: clean release

run_healthcenter:
	@export LD_LIBRARY_PATH=$(CURDIR)/build/lib:${LD_LIBRARY_PATH} && ./bin/healthcenter

run_hospital:
	@export LD_LIBRARY_PATH=$(CURDIR)/build/lib:${LD_LIBRARY_PATH} && ./bin/hospital ./data/hospitalA.txt ./data/hospitalB.txt ./data/hospitalC.txt

run_student:
	@export LD_LIBRARY_PATH=$(CURDIR)/build/lib:${LD_LIBRARY_PATH} && ./bin/student ./data/student1.txt ./data/student2.txt ./data/student3.txt ./data/student4.txt ./data/student5.txt

clean:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/hospital clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/healthcenter clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/student clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/common clean
	@rm -rf $(CURDIR)/bin
	@rm -rf $(CURDIR)/build/lib
	@rm -rf $(CURDIR)/build/obj
