#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

void framebufferSizeCallback(GLFWwindow *window, int width, int height){
    glViewport(0, 0, width, height);
}


void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}


char *readShaderFile(const char *fileName){
    FILE* file = fopen(fileName, "r");
    fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
    char *fileText = malloc(sizeof(char) * (size + 1));

    fread(fileText, 1, size,file);
    fileText[size] = '\0';

    fclose(file);
    return fileText;
}


unsigned int linkShaders(const char *vertexFileName, const char *fragmentFileName){
    const GLchar *vertexFileText = readShaderFile(vertexFileName);
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexFileText, NULL);
    glCompileShader(vertexShader);

    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success){
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",infoLog);
    }

    unsigned int fragmentShader;
    const GLchar *fragmentFileText = readShaderFile(fragmentFileName);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentFileText, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if(!success){
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",infoLog);
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if(!success){
        char infoLog[512];
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::LINKING_FAILED\n%s\n",infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if(window == NULL){
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("GLAD failed\n");
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    /*float triangleVertices[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        -0.0f, 0.5f, 0.0f,
    };*/

    //creates rect
    /*float vertices[] = {
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };*/

    //creates 2 triangles next to each other
    /*float vertices[] = {
        0.0f, 0.0f, 0.0f,
        -0.5f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        -0.25f, 0.5f, 0.0f,
        0.25f, 0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        0, 2, 4
    };*/

    float vertices[] = {
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.25f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    float vertices2[] = {
        0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.25f, 0.5f, 0.0f
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    //unsigned int EBO;
    //glGenBuffers(1, &EBO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);  //Creates buffer for array data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  //binds buffer to the GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);  //telling the openGL state box how to read the inputVector
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /*unsigned int VAO2, VBO2;
    glGenVertexArrays(2, &VAO2);
    glGenBuffers(2, &VBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);  //binds buffer to the GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);  //telling the openGL state box how to read the inputVector
    glEnableVertexAttribArray(0);*/

    unsigned int shaderProgram = linkShaders("vertex.glsl", "fragment.glsl");
    //unsigned int yellowShaderProgram = linkShaders("vertex.glsl", "yellowfrag.glsl");
    glUseProgram(shaderProgram);


    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Enables wireFrame mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // reverts to fill mode (default)

    while(!glfwWindowShouldClose(window)){
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        processInput(window);

        glUseProgram(shaderProgram);
        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);  // This is when you are not using the EBO
        //glUseProgram(yellowShaderProgram);
        //glBindVertexArray(VAO2);
        //glDrawArrays(GL_TRIANGLES, 0, 3);  // This is when you are not using the EBO
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //free(fileText);
    glfwTerminate();
    return 0;
}
