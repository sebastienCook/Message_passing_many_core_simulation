CC=gcc
CFLAGS=-g -I. -lpthread -lrt
DEPS = parser.tab.h ast.h display.h semantics.h ir_generator.h hr_interpreter.h code_generator.h code_interpreter.h code_output.h
OBJ = lex.yy.o parser.tab.o chc.o ast.o display.o semantics.o ir_generator.o hr_interpreter.o code_generator.o code_interpreter.o code_output.o

lex.yy.c:	scanner.l $(DEPS)
	flex scanner.l

parser.tab.c: parser.y
	bison -d parser.y

parser.tab.h: parser.y
	bison -d parser.y

parser.tab.o:	parser.tab.c
	$(CC) -c parser.tab.c $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chc: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) 
	rm -f *.o  lex.yy.c parser.tab.h parser.tab.c  
	
clean:
	rm -f *.o lex.yy.c parser.tab.h parser.tab.c chc chc_output.c output
