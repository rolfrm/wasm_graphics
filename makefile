OPT = -g0 -O3
LIB_SOURCES1 = main.c engine.c utils.c audio.c 
LEVEL_SOURCES = level1.data level2.data level3.data level4.data

LIB_SOURCES = $(addprefix src/, $(LIB_SOURCES1))
CC = gcc
TARGET = run.exe
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LEVEL_CS = $(addprefix src/, $(LEVEL_SOURCES:.data=.c))
LDFLAGS= -L. $(OPT) -Wextra 
LIBS= -lGL -lGLEW -lglfw -lm -liron -lopenal -licydb
ALL= $(TARGET)
CFLAGS = -Isrc/ -Iinclude/ -std=gnu11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -fdiagnostics-color -Wextra  -Wwrite-strings -Werror -msse4.2 -Werror=maybe-uninitialized -DUSE_VALGRIND -DDEBUG

$(TARGET): $(LIB_OBJECTS)
	$(CC) $(LDFLAGS) $(LIB_OBJECTS) $(LIBS) -o $@

all: $(ALL)

.c.o: $(HEADERS) $(LEVEL_CS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends
src/engine.o: src/flat_geom.shader.c src/starry.shader.c $(LEVEL_CS) 

src/flat_geom.shader.c: src/flat_geom.vs src/flat_geom.fs
	xxd -i src/flat_geom.vs > src/flat_geom.shader.c
	xxd -i src/flat_geom.fs >> src/flat_geom.shader.c
src/starry.shader.c: src/starry.vs src/starry.fs
	xxd -i src/starry.vs > src/starry.shader.c
	xxd -i src/starry.fs >> src/starry.shader.c
src/level%.c: level%.data
	xxd -i $< > $@

depend: h-depend
clean:
	rm -f $(LIB_OBJECTS) $(ALL) src/*.o.depends src/*.o src/level*.c src/*.shader.c 
.PHONY: test
test: $(TARGET)
	make -f makefile.compiler
	make -f makefile.test test

-include $(LIB_OBJECTS:.o=.o.depends)

