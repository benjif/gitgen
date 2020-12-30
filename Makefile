NAME = gitgen

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

HILI_DIR := /usr/share/source-highlight/

CFLAGS := -Wall -Wextra -std=c++20 -lgit2 \
	-L/usr/local/lib -Iinclude -I/usr/local/include -I. \
	-DFMT_HEADER_ONLY -O3

OBJ_FILES := src/gitgen.o \
	src/templates.o	\
	src/index.o	\
	src/repo.o

ifeq ($(GG_COLOR), TRUE)
CFLAGS := $(CFLAGS) `pkg-config --libs --cflags source-highlight` \
	-DHIGHLIGHT
OBJ_FILES := $(OBJ_FILES) src/color.o
endif

ifeq ($(GG_MARKDOWN), TRUE)
CFLAGS := -DMARKDOWN -lmd4c-html $(CFLAGS)
OBJ_FILES := $(OBJ_FILES) src/markdown.o
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
