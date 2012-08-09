obj = lex.o parser.o posix.o emit.o ins.o dump.o main.o util.o

compiler: $(obj) lib
	cc ${obj}  -o compiler

lex.o: util.h lex.h

parser.o:util.h lex.h parser.h

posix.o: util.h parser.h posix.h

emit.o: util.h ins.h parser.h emit.h

ins.o: util.h  parser.h ins.h

dump.o: util.h ins.h emit.h dump.h

main.o: util.h parser.h emit.h dump.h 

util.o: util.h

lib: io.c
	cc -c -fPIC -shared -o libio.so io.c

.PHONY: clean
clean:
	rm *.o compiler libio.so
