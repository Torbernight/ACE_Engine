CC=clang
CXX=clang++
GLS=glslang

GLSFLAGS=-G

DBGFLAGS=-g
RLSFLAGS=-s

CFLAGS:=-c -Wall $(DBGFLAGS) -MMD
CXXFLAGS:=-c -Wall $(DBGFLAGS) -MMD -fms-extensions

CSOURCE := $(wildcard src/*.c) $(wildcard src/example/*.c) 
CXXSOURCE := $(wildcard src/*.cpp) $(wildcard src/example/*.cpp) $(wildcard src/imgui/*.cpp) src/imgui/backends/imgui_impl_sdl2.cpp src/imgui/backends/imgui_impl_opengl3.cpp src/imgui/misc/cpp/imgui_stdlib.cpp
SOURCE = $(CSOURCE) $(CXXSOURCE)

INCLUDE = -Isrc/imgui -I/usr/include/SDL2

SHADER := $(wildcard shader/*.glsl)
SHADER_BIN = ${SHADER:.glsl=.spv}

COBJ = $(CSOURCE:.c=.o)
CXXOBJ = $(CXXSOURCE:.cpp=.o)
OBJ = $(COBJ) $(CXXOBJ)

DEPENDS = ${OBJ:.o=.d}

LIBS = -lGL -lGLEW -lSDL2 -lm -lfreeimageplus -lassimp

EXE= cpp_template

all: $(SOURCE) $(EXE)

$(EXE): $(OBJ)
		$(CXX) $(OBJ) $(LIBS) $(INCLUDE) -o $@

-include ${DEPENDS}

%.o: %.cpp
		$(CXX) $(CXXFLAGS) $(INCLUDE) $< -o $@

%.o: %.c
		$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

clean:
		rm -f $(OBJ) $(DEPENDS) $(EXE)

objclean:
		rm -f $(OBJ) $(DEPENDS)

shader: $(SHADER_BIN)

%.spv: %.glsl
		$(GLS) $(GLSFLAGS) $< -o $@

shaderclean:
	rm -f $(SHADER_BIN)