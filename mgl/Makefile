LEX = lex -I
YACC = yacc

CC = gcc -DYYDEBUG=1

mgl: y.tab.o lex.yy.o subr.o var_store.o
	$(CC) -o mgl y.tab.o lex.yy.o subr.o var_store.o -ly -ll

lex.yy.o: lex.yy.c y.tab.h

y.tab.o: y.tab.c y.tab.h
	$(CC) -c y.tab.c y.tab.h

subr.o: subr.c
	$(CC) -c subr.c

var_store.o: var_store.c
	$(CC) -c var_store.c

y.tab.c y.tab.h: mgl.y
	$(YACC) -d mgl.y

lex.yy.c: mgl.lex
	$(LEX) mgl.lex

clean:
	-rm *.o *.gch

distclean: clean
	-rm lex.yy.c y.tab.c y.tab.h mgl

