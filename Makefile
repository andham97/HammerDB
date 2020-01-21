cc = clang
target = none

all: table.o database.o main.o
	$(cc) -g -o db $? || make target=all clean

%.o: %.c
	$(cc) -g -c $<

test: test.o
	$(cc) -g -o test test.o

clean:
	rm -rf *.o
	make $(target)

mc: clean all
	sudo /usr/bin/valgrind -s --track-origins=yes --log-file="memcheck.txt" --tool=memcheck --leak-check=yes ./db

none: