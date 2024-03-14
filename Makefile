MAKEFLAGS := --jobs=$(shell nproc) --output-sync=target 

CXX := clang++ -stdlib=libc++ -fcolor-diagnostics -fansi-escape-codes -fdiagnostics-format=msvc
# we have to compile wayland glue code with c compiler due to linkage issues
CC := clang -fcolor-diagnostics -fansi-escape-codes -fdiagnostics-format=msvc

WARNINGS := -Wall -Wextra -Wpedantic -Wno-gnu-anonymous-struct -Wno-missing-braces -Wno-c99-designator

include debug.mk

PGKS := egl glesv2 wayland-client wayland-egl
PKG := $(shell pkg-config --cflags $(PGKS))
PKG_LIB := $(shell pkg-config --libs $(PGKS))

CXXFLAGS := -std=c++23 $(PKG)
CFLAGS := -std=c2x $(PGK)
LDFLAGS := $(PKG_LIB) -fuse-ld=lld

WAYLAND_PROTOCOLS_DIR := $(shell pkg-config wayland-protocols --variable=pkgdatadir)
WAYLAND_SCANNER := $(shell pkg-config --variable=wayland_scanner wayland-scanner)

XDG_SHELL := $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml
POINTER_CONSTRAINTS := $(WAYLAND_PROTOCOLS_DIR)/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml
RELATIVE_POINTER := $(WAYLAND_PROTOCOLS_DIR)/unstable/relative-pointer/relative-pointer-unstable-v1.xml

SRCD := .
BD := ./build
WLD := ./wayland
BIN := $(shell cat name)
EXEC := $(BD)/$(BIN)

SRCS := $(shell find $(SRCD) -name '*.cc')
OBJ := $(SRCS:%=$(BD)/%.o)

all: CXX += -flto=full $(SAFE_STACK) 
all: CC += -flto=full $(SAFE_STACK) 
all: CXXFLAGS += -g -O3 -march=native $(WARNING) -DNDEBUG
all: CFLAGS += -g -O3 -march=native $(WARNING) -DNDEBUG
all: $(EXEC)

debug: CXX += $(ASAN)
debug: CC += $(ASAN)
debug: CXXFLAGS += -g -O0 $(DEBUG) $(WARNINGS) $(WNO)
debug: CFLAGS += -g -O0 $(DEBUG) $(WARNINGS) $(WNO)
debug: $(EXEC)

# rules to build everything
$(EXEC): $(OBJ) $(BD)/xdg-shell.c.o $(BD)/pointer-constraints-unstable-v1.c.o $(BD)/relative-pointer-unstable-v1.c.o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BD)/%.cc.o: %.cc headers/* Makefile debug.mk $(BD)/xdg-shell.c.o $(BD)/pointer-constraints-unstable-v1.c.o $(BD)/relative-pointer-unstable-v1.c.o
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BD)/%.c.o: $(WLD)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(WLD)/xdg-shell.h:
	mkdir -p $(WLD)
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL) $@ 

$(WLD)/xdg-shell.c: $(WLD)/xdg-shell.h
	mkdir -p $(WLD)
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL) $@

$(WLD)/pointer-constraints-unstable-v1.h:
	mkdir -p $(WLD)
	$(WAYLAND_SCANNER) client-header $(POINTER_CONSTRAINTS) $@ 

$(WLD)/pointer-constraints-unstable-v1.c: $(WLD)/pointer-constraints-unstable-v1.h
	mkdir -p $(WLD)
	$(WAYLAND_SCANNER) private-code $(POINTER_CONSTRAINTS) $@ 

$(WLD)/relative-pointer-unstable-v1.h:
	mkdir -p $(WLD)
	$(WAYLAND_SCANNER) client-header $(RELATIVE_POINTER) $@ 

$(WLD)/relative-pointer-unstable-v1.c: $(WLD)/relative-pointer-unstable-v1.h
	mkdir -p $(WLD)
	$(WAYLAND_SCANNER) private-code $(RELATIVE_POINTER) $@ 

.PHONY: clean
clean:
	rm -rf $(BD) $(WLD)
