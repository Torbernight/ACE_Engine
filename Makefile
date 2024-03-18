CC=clang
CXX=clang++
GLS=glslang

GLSFLAGS=-G

DBGFLAGS=-g
RLSFLAGS=-s

CFLAGS:=-c -Wall $(DBGFLAGS) -MMD
CXXFLAGS:=-c -Wall $(DBGFLAGS) -MMD -fms-extensions

CSOURCE := $(wildcard src/*.c)
CXXSOURCE := $(wildcard src/*.cpp)
SOURCE = $(CSOURCE) $(CXXSOURCE)

SHADER := $(wildcard shader/*.glsl)
SHADER_BIN = ${SHADER:.glsl=.spv}

COBJ = $(CSOURCE:.c=.o)
CXXOBJ = $(CXXSOURCE:.cpp=.o)
OBJ = $(COBJ) $(CXXOBJ)

DEPENDS = ${OBJ:.o=.d}

LIBS = -lGL -lGLEW -lSDL2 -lm

EXE= cpp_template

all: $(SOURCE) $(EXE)

$(EXE): $(OBJ)
		$(CXX) $(OBJ) $(LIBS) -o $@

-include ${DEPENDS}

%.o: %.cpp
		$(CXX) $(CXXFLAGS) $< -o $@

%.o: %.c
		$(CC) $(CFLAGS) $< -o $@

clean:
		rm -f $(OBJ) $(DEPENDS) $(EXE)

objclean:
		rm -f $(OBJ) $(DEPENDS)

shader: $(SHADER_BIN)

%.spv: %.glsl
		$(GLS) $(GLSFLAGS) $< -o $@

shaderclean:
	rm -f $(SHADER_BIN)