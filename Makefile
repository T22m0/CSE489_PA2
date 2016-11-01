PA2: main.o shell.o
	gcc -lm -o PA2 ./obj/main.o ./obj/shell.o
main.o : ./src/main.c ./inc/main.h 
	gcc -c ./src/main.c -o ./obj/main.o
shell.o: ./src/main.c ./inc/main.h
	gcc -c ./src/shell.c -o ./obj/shell.o
clean:
	rm -f ./obj/* ./PA2
