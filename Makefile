MAKEFLAGS := --jobs=$(shell nproc) --output-sync=target 

CXX := clang++ -fcolor-diagnostics -fansi-escape-codes -fdiagnostics-format=msvc
# compile wayland glue code with c compiler due to linkage issues
CC := clang -fcolor-diagnostics -fansi-escape-codes -fdiagnostics-format=msvc

WARNINGS := -Wall -Wextra -fms-extensions -Wno-missing-field-initializers

include debug.mk

PKGS := egl glesv2 wayland-client wayland-egl wayland-cursor
PKG := $(shell pkg-config --cflags $(PKGS))
PKG_LIB := $(shell pkg-config --libs $(PKGS))

CXXFLAGS := -std=gnu++23 $(PKG)
CFLAGS := -std=gnu2x $(PKG)
LDFLAGS := $(PKG_LIB)

WAYLAND_PROTOCOLS_DIR := $(shell pkg-config wayland-protocols --variable=pkgdatadir)
WAYLAND_SCANNER := $(shell pkg-config --variable=wayland_scanner wayland-scanner)

XDG_SHELL := $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml
POINTER_CONSTRAINTS := $(WAYLAND_PROTOCOLS_DIR)/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml
RELATIVE_POINTER := $(WAYLAND_PROTOCOLS_DIR)/unstable/relative-pointer/relative-pointer-unstable-v1.xml

SRCD := .
BD := ./build
WLD := ./platform/wayland/
WLPD := $(WLD)/wayland-protocols
BIN := $(shell cat name)
EXEC := $(BD)/$(BIN)

SRCS := $(shell find $(SRCD) -name '*.cc' -not -path './platform/windows/*')
OBJ := $(SRCS:%=$(BD)/%.o)

gcc: CXX = g++ -fdiagnostics-color -flto=auto $(SAFE_STACK) -DFPS_COUNTER
gcc: CC = gcc -fdiagnostics-color -flto=auto $(SAFE_STACK) 
gcc: CXXFLAGS += -g -O3 -march=native -ffast-math $(WARNINGS) -DNDEBUG
gcc: CFLAGS += -g -O3 -march=native -ffast-math $(WARNINGS) -DNDEBUG
gcc: $(EXEC)

all: CXX += -flto=auto $(SAFE_STACK) -DFPS_COUNTER
all: CC += -flto=auto $(SAFE_STACK) 
all: CXXFLAGS += -g -O3 -march=native -ffast-math $(WARNINGS) -DNDEBUG
all: CFLAGS += -g -O3 -march=native -ffast-math $(WARNINGS) -DNDEBUG
all: $(EXEC)

debug: CXX += $(ASAN)
debug: CC += $(ASAN)
debug: CXXFLAGS += -g -O0 $(DEBUG) $(WARNINGS) $(WNO)
debug: CFLAGS += -g -O0 $(DEBUG) $(WARNINGS) $(WNO)
debug: $(EXEC)

$(EXEC): $(OBJ) $(BD)/xdg-shell.c.o $(BD)/pointer-constraints-unstable-v1.c.o $(BD)/relative-pointer-unstable-v1.c.o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BD)/%.cc.o: %.cc headers/* Makefile debug.mk $(BD)/xdg-shell.c.o $(BD)/pointer-constraints-unstable-v1.c.o $(BD)/relative-pointer-unstable-v1.c.o
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BD)/%.c.o: $(WLPD)/%.c $(WLD)/*
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(WLPD)/xdg-shell.h:
	mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL) $@ 

$(WLPD)/xdg-shell.c: $(WLPD)/xdg-shell.h
	mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL) $@

$(WLPD)/pointer-constraints-unstable-v1.h:
	mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) client-header $(POINTER_CONSTRAINTS) $@ 

$(WLPD)/pointer-constraints-unstable-v1.c: $(WLPD)/pointer-constraints-unstable-v1.h
	mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) private-code $(POINTER_CONSTRAINTS) $@ 

$(WLPD)/relative-pointer-unstable-v1.h:
	mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) client-header $(RELATIVE_POINTER) $@ 

$(WLPD)/relative-pointer-unstable-v1.c: $(WLPD)/relative-pointer-unstable-v1.h
	mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) private-code $(RELATIVE_POINTER) $@ 

.PHONY: clean tags
clean:
	rm -rf $(BD) $(WLPD) tags

tags:
	ctags -R --language-force=C++ --extras=+q+r --c++-kinds=+p+l+x+L+A+N+U+Z
