# edit the following line if you want to put rmutt elsewhere. the
# binary will be installed in $PREFIX/bin and the included .rm files
# will be installed in $PREFIX/share/rmutt
PREFIX=/usr/local

CC = gcc
CFLAGS = -g -Wall -pedantic -DRMUTT_INCLUDE=\"$(PREFIX)/share/rmutt\"
# -DDEBUG

OBJ = lex.yy.o rmutt.tab.o grambit.o list.o main.o grammar.o gstr.o rxutil.o dict.o choose.o
EXE = rmutt

LIB_DIR = 
# comment out the next line to compile under cygwin
LIBS = -lfl
# uncomment the next line to compile under cygwin
# LIBS = -lfl -lregex

all: $(OBJ) $(EXE)

clean:
	rm -rf *.o lex.yy.c rmutt.tab.c rmutt.tab.h core *~ $(EXE)

pure: $(EXE) $(OBJ)
	purify $(CC) $(OBJ) -o $(EXE) $(LIB_DIR) $(LIBS)

$(EXE): $(OBJ) lex.yy.c rmutt.tab.c
	$(CC) $(OBJ) -o $(EXE) $(LIB_DIR) $(LIBS)

lex.yy.c: rmutt.lex rmutt.tab.c
	flex rmutt.lex

rmutt.tab.c: rmutt.y
	bison -d rmutt.y

test: $(EXE)
	(cd test; sh runtests.sh)

install: $(EXE)
	cp $(EXE) $(PREFIX)/bin/$(EXE)
	mkdir -p $(PREFIX)/share/rmutt
	cp examples/*.rm $(PREFIX)/share/rmutt
