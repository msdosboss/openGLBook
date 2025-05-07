all: helloworld.c glad.c
	gcc -o main camera.c glad.c -lglfw -lm
