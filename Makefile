CC          ?= gcc
LN          ?= ln
MKDIR       ?= mkdir
RM          ?= rm
ECHO        ?= echo
TOUCH       ?= touch
PYTEST      ?= pytest

sostem      := libcmock
sover       := 0
socompat    := 0

cext        := c
oext        := o
soext       := so

builddir    := build
srcdir      := $(patsubst lib%,%,$(sostem))
testdir     := test

target      := $(sostem).$(soext).$(sover)
link        := $(sostem).$(soext)

config      := $(srcdir)/cmock_config.h

MOCKUPS     ?= $(abspath $(srcdir)/mockups.h)

CFLAGS      ?= -std=c11 -Wall -Wextra -Wpedantic -fPIC -c
CPPFLAGS    ?= -DCMOCK_LIBC=$(shell ldd /usr/bin/env | grep -oP "\s*\K/.*libc\.so(\.\d+)?")
LDFLAGS     ?= -shared -Wl,-soname,$(sostem).$(soext).$(socompat)
LDLIBS      ?= -ldl
LNFLAGS     ?= -sf
MKDIRFLAGS  ?= -p
RMFLAGS     ?= -rf
ECHOFLAGS   ?= -e
PYTESTFLAGS ?= -v --rootdir=$(testdir)

QUIET       ?= @

help        := .help.stamp

src         := $(wildcard $(srcdir)/*.$(cext))
obj         := $(patsubst $(srcdir)/%.$(cext),$(builddir)/%.$(oext),$(src))

.PHONY: all
all: $(target) $(link)

$(link): $(target)
	$(info [LN]  $@)
	$(QUIET)$(LN) $(LNFLAGS) $(abspath $(CURDIR))/$^ $@

$(target): $(obj)
	$(info [LD]  $@)
	$(QUIET)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(builddir)/%.$(oext): $(srcdir)/%.$(cext) $(config) | $(builddir)
	$(info [CC]  $@)
	$(QUIET)$(CC) -o $@ $(filter-out $(config),$^) $(CFLAGS) $(CPPFLAGS)

$(config): $(MOCKUPS)
	$(info [GEN] $@)
	$(QUIET)$(ECHO) $(ECHOFLAGS) '#ifndef CMOCK_CONFIG_H\n#define CMOCK_CONFIG_H\n#include "$^"\n#endif' > $@

.PHONY: test
test:
	$(QUIET)$(PYTEST) $(PYTESTFLAGS)

$(builddir):
	$(QUIET)$(MKDIR) $(MKDIRFLAGS) $@

.PHONY: clean
clean:
	$(QUIET)$(RM) $(RMFLAGS) $(builddir) $(target) $(link) $(help) $(config)

$(help): FORCE
	$(info +=========+)
	$(info |  cmock  |)
	$(info +=========+)
	$(info Linker-based function mocking)
	$(info )
	$(info Building:)
	$(info Create $(MOCKUPS) and specify which functions to mock)
	$(info Run 'make' (without any targets))
	$(info The path to mockups file may be modified by passing the MOCKUPS variable)
	$(info )
	$(info Testing (requires pytest):)
	$(info Run 'make test')
	$(info )
	$(info For more info: gitlab.com/vilhelmengstrom/cmock)
	$(QUIET)$(TOUCH) $@

.PHONY: help
help: $(help)

FORCE:
