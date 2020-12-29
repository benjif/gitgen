NAME = gitgen

CFLAGS := -Wall -Wextra -std=c++20 -lgit2 \
	-L/usr/local/lib -Iinclude -I/usr/local/include -I. \
	-D FMT_HEADER_ONLY -O2
OBJ_FILES :=	src/gitgen.o \
				src/templates.o	\
				src/index.o	\
				src/repo.o

all: release
.PHONY: all

release:
	$(MAKE) clean
	$(MAKE) gitgen

%.o: %.cpp
	$(CXX) -c $^ -o $@ $(CFLAGS)

gitgen: $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $@ $(CFLAGS)

clean:
	find . -name "*.o" -type f -delete
	rm -f gitgen
