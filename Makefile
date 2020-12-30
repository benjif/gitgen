NAME = gitgen

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

HILI_DIR := /usr/share/source-highlight/

BASEFLAGS := -Wall -Wextra -std=c++20 -lgit2 \
	-L/usr/local/lib -Iinclude -I/usr/local/include -I. \
	-D FMT_HEADER_ONLY -g #-O2

BASE_OBJ_FILES := src/gitgen.o \
	src/templates.o	\
	src/index.o	\
	src/repo.o

ifeq ($(COLOR), TRUE)
CFLAGS := $(BASEFLAGS) `pkg-config --libs --cflags source-highlight` \
	-D HIGHLIGHT
OBJ_FILES := $(BASE_OBJ_FILES) src/color.o
else
CFLAGS := $(BASEFLAGS)
OBJ_FILES := $(BASE_OBJ_FILES)
endif

release:
	$(MAKE) clean
	$(MAKE) gitgen

all: release
.PHONY: all

%.o: %.cpp
	$(CXX) -c $^ -o $@ $(CFLAGS)

gitgen: $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $@ $(CFLAGS)

install-color:
ifneq ($(shell id -u), 0)
	@echo "*** install-color requires root to install source-highlight files."
	exit 1
else
	cp source-highlight/html_gitgen.outlang $(HILI_DIR)
endif

install:
	install gitgen $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/gitgen
	rm -f $(HILI_DIR)/html_gitgen.outlang

clean:
	find . -name "*.o" -type f -delete
	rm -f gitgen
