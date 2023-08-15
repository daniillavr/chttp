TEMP_DIR=build
VER=0.1

all: clean_out shared static
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
