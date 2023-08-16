TEMP_DIR=build
VER=0.1

all: test_build_win
debug: clean_out shared_debug static_debug

shared:
	mkdir -p $(TEMP_DIR)/out
	gcc -Wall -Werror -fpic -o $(TEMP_DIR)/http.o -c http.c
	gcc -shared -o $(TEMP_DIR)/out/libhttp_$(VER).so $(TEMP_DIR)/http.o
static:
	mkdir -p $(TEMP_DIR)/out
	gcc -Wall -Werror -o $(TEMP_DIR)/libhttp.o -c http.c
	ar rcs $(TEMP_DIR)/out/libhttp_$(VER).a $(TEMP_DIR)/http.o
shared_debug:
	mkdir -p $(TEMP_DIR)/out
	gcc -g -Wall -Werror -fpic -DDEBUG -o $(TEMP_DIR)/http.o -c http.c
	gcc -shared -o $(TEMP_DIR)/out/libhttp_$(VER).so $(TEMP_DIR)/http.o
static_debug:
	mkdir -p $(TEMP_DIR)/out
	gcc -g -Wall -Werror -DDEBUG -o $(TEMP_DIR)/libhttp.o -c http.c
	ar rcs $(TEMP_DIR)/out/libhttp_$(VER).a $(TEMP_DIR)/http.o
clean_out:
	rm -rf build/out
clean_build:
	rm -rf build

test_build_win:
	-mkdir $(TEMP_DIR)\out
	gcc -g -Wall -c -o $(TEMP_DIR)/http.o http.c
	gcc -g -Wall -c -o $(TEMP_DIR)/main.o main.c
	gcc -o main $(TEMP_DIR)/main.o $(TEMP_DIR)/http.o
