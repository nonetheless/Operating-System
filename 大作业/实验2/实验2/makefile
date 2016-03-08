out:	fat12.c	foo.o
	gcc -m32 -Wno-implicit -o out foo.o fat12.c
	./out
foo.o:	foo.asm
	./nasm -f macho32 -o foo.o foo.asm
clean:
	rm foo.o out
out1:	main.c	foo.o
	gcc -m32 -Wno-implicit -o out1 foo.o main.c