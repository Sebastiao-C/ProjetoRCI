ring: ring.o ringaux.o 
	gcc -g -fdiagnostics-color=always -Wall -o ring ring.o ringaux.o

ring.o: ring.c
	gcc -g -fdiagnostics-color=always -Wall -c ring.c

ringaux.o: ringaux.h
	gcc -g -fdiagnostics-color=always -Wall -c ringaux.c

ring.c: ringaux.h

clean:
	rm *.o ring
