NAME = gitgen

CFLAGS := -Wall -Wextra -O2 -std=c++20 -lgit2 \
	-L/usr/local/lib -Iinclude -I/usr/local/include -I. \
	-D FMT_HEADER_ONLY
OBJ_FILES :=	src/gitgen.o \
				src/templates.o	\
				src/index.o	\
				src/repo.o

all: clean $(OBJ_FILES) gitgen
.PHONY: all

%.o: %.cpp
	$(CXX) -c $^ -o $@ $(CFLAGS)

gitgen: $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $@ $(CFLAGS)

clean:
	find . -name "*.o" -type f -delete
	rm -f gitgen
