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

__histogramd: __commond
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/histogram debug

__histogram: __common
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/histogram release

__quantizerd: __commond
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/quantizer debug

__quantizer: __common
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/quantizer release

__blockd: __commond
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/block debug

__block: __common
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/block release

__stdvqlibd:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/stdvqlib debug

__stdvqlib:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/stdvqlib release

__stdvqd: __stdvqlibd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/stdvq debug

__stdvq: __stdvqlib
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/stdvq release

__stdvqed: __stdvqlibd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/stdvqe debug

__stdvqe: __stdvqlib
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/stdvqe release

__tsvqlibd:
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/tsvqlib debug

__tsvqlib:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/tsvqlib release

__tsvqd: __tsvqlibd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/tsvq debug

__tsvq: __tsvqlib
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/tsvq release

__tsvqed: __tsvqlibd
	$(MAKE) $(MAKE_FLAGS) $(DEBUG_MAKE_FLAGS) -C $(CURDIR)/tsvqe debug

__tsvqe: __tsvqlib
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/tsvqe release

debug: __commond __histogramd __quantizerd __blockd \
	__stdvqlibd __stdvqd __stdvqed __tsvqlibd __tsvqd __tsvqed

release: __common __histogram __quantizer __block \
	__stdvqlib __stdvq __stdvqe __tsvqlib __tsvq __tsvqe

all: debug release

default: release

clean:
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/histogram clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/quantizer clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/block clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/stdvq clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/stdvqe clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/tsvq clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/tsvqe clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/common clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/stdvqlib clean
	@$(MAKE) $(MAKE_FLAGS) $(RELEASE_MAKE_FLAGS) -C $(CURDIR)/tsvqlib clean
	@rm -rf $(CURDIR)/bin
	@rm -rf $(CURDIR)/build/lib
	@rm -rf $(CURDIR)/build/obj
