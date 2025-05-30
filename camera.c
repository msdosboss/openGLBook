#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "cglm/cglm.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct Camera{
    vec3 position;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
    float sensitivity;
    float lastX, lastY;
    int firstMouse;
    float fov;
};


struct CubePositions{
    vec3 *cubePositions;
    int positionIndex;
};


struct CamAndPos{
    struct Camera *cam;
    struct CubePositions *cubePosition;
};


void framebufferSizeCallback(GLFWwindow *window, int width, int height){
    glViewport(0, 0, width, height);
}


void processInput(GLFWwindow *window, float deltaTime){
    struct CamAndPos *camAndPos = glfwGetWindowUserPointer(window);
    struct Camera *cam = camAndPos->cam;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    vec3 cameraSpeed = {2.5f * deltaTime, 2.5f * deltaTime, 2.5f * deltaTime};

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_vec3_mul(cameraSpeed, cam->front, cameraPosOffset);
        float oldCamYposition = cam->position[1];
        glm_vec3_add(cam->position, cameraPosOffset, cam->position);
        //cam->position[1] = oldCamYposition;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_vec3_mul(cameraSpeed, cam->front, cameraPosOffset);
        float oldCamYposition = cam->position[1];
        glm_vec3_sub(cam->position, cameraPosOffset, cam->position);
        //cam->position[1] = oldCamYposition;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_cross(cam->front, cam->up, cameraPosOffset);
        glm_normalize(cameraPosOffset);
        glm_vec3_mul(cameraPosOffset, cameraSpeed, cameraPosOffset);
        glm_vec3_sub(cam->position, cameraPosOffset, cam->position);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_cross(cam->front, cam->up, cameraPosOffset);
        glm_normalize(cameraPosOffset);
        glm_vec3_mul(cameraPosOffset, cameraSpeed, cameraPosOffset);
        glm_vec3_add(cam->position, cameraPosOffset, cam->position);
    }
}


char *readShaderFile(const char *fileName){
    FILE* file = fopen(fileName, "r");
    if(file == NULL){
        printf("uhhhhhhhhhhhhhhhhhhhhh he's right behind isn't he?\n");
    }
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


unsigned int *genTextures(const char *firstFileName, const char *secondFileName){

    stbi_set_flip_vertically_on_load(true);

    unsigned int *textures = malloc(sizeof(unsigned int) * 2);

    int width, height, nChannels;
    unsigned char *data = stbi_load(firstFileName, &width, &height, &nChannels, 0);

    glGenTextures(1, &textures[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    data = stbi_load(secondFileName, &width, &height, &nChannels, 0);

    glGenTextures(1, &textures[1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return textures;
}


void mouseCallback(GLFWwindow *window, double xpos, double ypos){
    struct CamAndPos *camAndPos = glfwGetWindowUserPointer(window);
    struct Camera *cam = camAndPos->cam;

    if(cam->firstMouse){
        cam->lastX = xpos;
        cam->lastY = ypos;
        cam->firstMouse = 0;
    }

    float xoffset = xpos - cam->lastX;
    float yoffset = cam->lastY - ypos;
    cam->lastX = xpos;
    cam->lastY = ypos;

    xoffset *= cam->sensitivity;
    yoffset *= cam->sensitivity;

    cam->yaw += xoffset;
    cam->pitch += yoffset;

    if(cam->pitch > 89.0f){
        cam->pitch = 89.0f;
    }
    if(cam->pitch < -89.0f){
        cam->pitch = -89.0f;
    }

    vec3 directions;
    directions[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    directions[1] = sin(glm_rad(cam->pitch));
    directions[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));

    glm_normalize_to(directions, cam->front);
}


void scrollCallback(GLFWwindow *window, double xoffset, double yoffset){
    struct CamAndPos *camAndPos = glfwGetWindowUserPointer(window);
    struct Camera *cam = camAndPos->cam;

    cam->fov -= (float)yoffset;
    if(cam->fov < 1.0f){
        cam->fov = 1.0f;
    }
    if(cam->fov > 89.0f){
        cam->fov = 89.0f;
    }
}


void addCube(GLFWwindow *window, vec3 newCubePosition){
    struct CamAndPos *camAndPos = glfwGetWindowUserPointer(window);
    struct CubePositions *cubePosition = camAndPos->cubePosition;
    cubePosition->cubePositions = (vec3*)realloc(cubePosition->cubePositions, sizeof(vec3) * (cubePosition->positionIndex + 1));
    cubePosition->cubePositions[cubePosition->positionIndex][0] = newCubePosition[0];
    cubePosition->cubePositions[cubePosition->positionIndex][1] = newCubePosition[1];
    cubePosition->cubePositions[cubePosition->positionIndex++][2] = newCubePosition[2];
}


void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods){
    struct CamAndPos *camAndPos = glfwGetWindowUserPointer(window);
    struct Camera *cam = camAndPos->cam;
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        vec3 cameraCenter;
        glm_vec3_add(cam->position, cam->front, cameraCenter);
        cameraCenter[0] = (int)cameraCenter[0];
        cameraCenter[1] = (int)cameraCenter[1];
        cameraCenter[2] = (int)cameraCenter[2];
        addCube(window, cameraCenter);
    }
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    vec3 intialCubePositions[16];

    for(int i = 0; i < 16; i++){
        intialCubePositions[i][0] = i / 4;
        intialCubePositions[i][1] = 0;
        intialCubePositions[i][2] = i % 4;
    }

    struct CubePositions cubePosition = {
        .cubePositions = malloc(sizeof(vec3) * 16),
        .positionIndex = 0
    };

    for(; cubePosition.positionIndex < 16; cubePosition.positionIndex++){
        cubePosition.cubePositions[cubePosition.positionIndex][0] = intialCubePositions[cubePosition.positionIndex][0];
        cubePosition.cubePositions[cubePosition.positionIndex][1] = intialCubePositions[cubePosition.positionIndex][1];
        cubePosition.cubePositions[cubePosition.positionIndex][2] = intialCubePositions[cubePosition.positionIndex][2];
    }

    vec3 rotations[] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {0.5f, 0.0f, 0.5f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
    };

    //creates cube
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);  //Creates buffer for array data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  //binds buffer to the GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);  //telling the openGL state box how to read the inputVector
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int shaderProgram = linkShaders("shaders/3dVertex.glsl", "shaders/3dFragments.glsl");
    glUseProgram(shaderProgram);

    float mixValue = 0.2;

    glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);

    unsigned int *textures = genTextures("textures/wall.jpg", "textures/awesomeface.png");
    glBindVertexArray(VAO);

    int textureUniformLocation = glGetUniformLocation(shaderProgram, "ourTexture");
    glUniform1i(textureUniformLocation, 0);

    glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

    glEnable(GL_DEPTH_TEST);

    struct Camera camera ={
        .position = {0.0f, 0.0f, 3.0f},
        .front = {0.0f, 0.0f, -1.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .yaw = -90.0f,
        .pitch = 0.0f,
        .sensitivity = 0.1f,
        .lastX = 400.0f,
        .lastY = 300.0f,
        .firstMouse = 1,
        .fov = 45.0f
    };
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    struct CamAndPos camAndPos = {
        .cam = &camera,
        .cubePosition = &cubePosition
    };

    glfwSetWindowUserPointer(window, &camAndPos);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, deltaTime);

        glUseProgram(shaderProgram);

        mat4 view;

        vec3 cameraCenter;
        glm_vec3_add(camera.position, camera.front, cameraCenter);
        glm_lookat(camera.position, cameraCenter, camera.up, view);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (const float *)view);

        mat4 projection;
        glm_perspective(glm_rad(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (const float *)projection);

        for(int i = 0; i < cubePosition.positionIndex; i++){
            mat4 model;
            glm_mat4_identity(model);
            glm_translate(model, cubePosition.cubePositions[i]);
            //vec3 rotationAxises = {1.0f, -1.0f, 1.0f};
            //glm_rotate(model, (float)glfwGetTime() * glm_rad(50.0f), rotations[i % 10]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (const float *)model);

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    free(cubePosition.cubePositions);
    glfwTerminate();
    return 0;
}
