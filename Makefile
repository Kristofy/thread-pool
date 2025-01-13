CXX = g++

CXX_COMMON_FLAGS  = --std=c++17 -xc++ -pipe -Wall -Wextra -Wshadow -fmax-errors=2 -Wconversion -Wsign-conversion -Wunreachable-code -Wpedantic -mcx16 
CXX_DEBUG_FLAGS   = -O0 -ggdb3 -g $(if $(asan), -fsanitize=address)
CXX_RELEASE_FLAGS = -O2
CXX_FLAGS         = $(CXX_COMMON_FLAGS) $(if $(release), $(CXX_RELEASE_FLAGS), $(CXX_DEBUG_FLAGS))

CXX_COMMON_DEFINES = -D_GNU_SOURCE \
		     -D_REENTRANT \
		     -D__STDC_CONSTANT_MACROS \
		     -D__STDC_FORMAT_MACROS \
		     -D__STDC_LIMIT_MACROS

CXX_DEBUG_DEFINES = -D_GLIBCXX_ASSERTIONS -DDEBUG 
CXX_RELEASE_DEFINES = 
CXX_DEFINES = $(CXX_COMMON_DEFINES) $(if $(CXX_RELEASE_DEFINES), $(CXX_RELEASE_DEFINES), $(CXX_DEBUG_DEFINES))
CXX_LINK = -lm -lpthread

# Every source file except "main.cpp" and "tests.cpp" with recursive globbing and not uder "/dev"
sources = $(shell \
	find . \
	-type f \
	\( -name "*.hpp" -o -name "*.h" \) \
	! -path "./dev/*")

main: $(sources) main.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_DEFINES) $(CXX_LINK) main.cpp -o main

run: main
	./main

btest: tests.cpp $(sources)
	$(CXX) $(CXX_FLAGS) $(CXX_DEFINES) $(CXX_LINK) tests.cpp -o test

test: tests.cpp $(sources)
	$(CXX) $(CXX_FLAGS) $(CXX_DEFINES) $(CXX_LINK) tests.cpp -o test
	./test

configure: main.cpp $(sources)
	bear -- $(CXX) $(CXX_FLAGS) $(CXX_DEFINES) $(CXX_LINK) $(sources) -o main

clean:
	rm -f main test

.PHONY: run clean configure
