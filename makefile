all: helloworld.c glad.c
	gcc -o main textures.c glad.c -lglfw -lm
