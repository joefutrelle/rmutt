CC = gcc
CFLAGS = -g -Wall -pedantic
#-DDEBUG
OBJ = lex.yy.o dada.tab.o grambit.o list.o main.o grammar.o gstr.o rxutil.o dict.o
EXE = rmutt

LIB_DIR = 
# comment out the next line to compile under cygwin
LIBS = -lfl
# uncomment the next line to compile under cygwin
# LIBS = -lfl -lregex

all: $(OBJ) $(EXE)

clean:
	rm -rf *.o lex.yy.c dada.tab.c dada.tab.h dada.c core *~ $(EXE)

pure: $(EXE) $(OBJ)
	purify $(CC) $(OBJ) -o $(EXE) $(LIB_DIR) $(LIBS)

$(EXE): $(OBJ) lex.yy.c dada.tab.c
	$(CC) $(OBJ) -o $(EXE) $(LIB_DIR) $(LIBS)

lex.yy.c: dada.lex dada.tab.c
	flex dada.lex

dada.tab.c: dada.y
	bison -d dada.y

install: $(EXE)
	cp $(EXE) /usr/local/bin/$(EXE)
