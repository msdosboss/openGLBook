all: helloworld.c glad.c
	gcc -o main helloworld.c glad.c -lglfw
