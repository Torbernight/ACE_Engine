CC=clang
CXX=clang++

DBGFLAGS=-g
RLSFLAGS=-s

CFLAGS:=-c -Wall $(DBGFLAGS) -MMD
CXXFLAGS:=-c -Wall $(DBGFLAGS) -MMD

CSOURCE := $(wildcard src/*.c)
CXXSOURCE := $(wildcard src/*.cpp)
SOURCE = $(CSOURCE) $(CXXSOURCE)

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
		rm -rf $(OBJ) $(DEPENDS) $(EXE)

objclean:
		rm -rf $(OBJ) $(DEPENDS)