CC=clang
CXX=clang++
GLS=glslang

GLSFLAGS=-G
DBGFLAGS=-g
RLSFLAGS=-O3 -DNDEBUG

CFLAGS:=-c -Wall $(DBGFLAGS) -MMD
CXXFLAGS:=-c -Wall $(DBGFLAGS) -MMD -fms-extensions

CSOURCE := $(wildcard src/*.c) $(wildcard src/example/*.c) 
CXXSOURCE := $(wildcard src/*.cpp) $(wildcard src/example/*.cpp) $(wildcard src/imgui/*.cpp) src/imgui/backends/imgui_impl_sdl2.cpp src/imgui/backends/imgui_impl_opengl3.cpp src/imgui/misc/cpp/imgui_stdlib.cpp
SOURCE = $(CSOURCE) $(CXXSOURCE)

INCLUDE = -Isrc/imgui -I/usr/include/SDL2
SHADER_INCLUDE = -Ishader/include

SHADER := $(wildcard shader/*.glsl)
SHADER_BIN = ${SHADER:.glsl=.spv}
#SHADER_BIN = ${SHADER:.glsl=.h}

COBJ = $(CSOURCE:.c=.o)
CXXOBJ = $(CXXSOURCE:.cpp=.o)
OBJ = $(COBJ) $(CXXOBJ)

DEPENDS = ${OBJ:.o=.d}

LIBS = -lGL -lGLEW -lSDL2 -lm -lfreeimageplus -lassimp -ltbb

EXE= cpp_template.out

all: $(SOURCE) $(EXE)

$(EXE): $(OBJ)
		$(CXX) $(OBJ) $(LIBS) $(INCLUDE) -pg -o $@

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
		$(GLS) $(GLSFLAGS) $(SHADER_INCLUDE) $< -o $@
#%.h: %.glsl
#		$(GLS) $(GLSFLAGS) $(SHADER_INCLUDE) --vn $(subst shader/,,$(subst .,_,$(subst .glsl,, $<))) $< -o $@

shaderclean:
	rm -f $(SHADER_BIN)