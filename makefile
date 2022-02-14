CC=gcc
CFLAGS=-g -I. -lpthread -lrt -lm
SUBDIR=Data

DEPS = chc_compiler/parser.tab.h chc_compiler/ast.h chc_compiler/display.h chc_compiler/semantics.h chc_compiler/ir_generator.h chc_compiler/hr_interpreter.h chc_compiler/code_generator.h chc_compiler/code_interpreter.h chc_compiler/code_output.h cpu.h

OBJH = many_core.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sim: $(OBJH)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o sim *.txt
	(cd Data; rm -f *.o sim *.txt)
	(cd chc_compiler; make clean;)
