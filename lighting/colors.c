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
    free((char *)vertexFileText);

    unsigned int fragmentShader;
    const GLchar *fragmentFileText = readShaderFile(fragmentFileName);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentFileText, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    free((char *)fragmentFileText);

    if(!success){
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",infoLog);
        return 0;
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
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


unsigned int genTexture(const char *firstFileName, GLenum textureCount){

    stbi_set_flip_vertically_on_load(true);

    unsigned int texture;

    int width, height, nChannels;
    unsigned char *data = stbi_load(firstFileName, &width, &height, &nChannels, 0);
    if(data == NULL){
        printf("ERROR: Failed to laod texture %s\n", firstFileName);
        return 0;
    }

    glGenTextures(1, &texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glActiveTexture(textureCount);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format = GL_RGB;
    if(nChannels == 1) format = GL_RED;
    else if(nChannels == 4) format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
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
        //0-2 position, 3-5 normals, 6-7 texture cords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);  //Creates buffer for array data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  //binds buffer to the GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);  //telling the openGL state box how to read the inputVector
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);


    unsigned int shaderProgram = linkShaders("shaders/colorVertex.glsl", "shaders/colorFragments.glsl");
    glUseProgram(shaderProgram);

    unsigned int lightShaderProgram = linkShaders("shaders/lightVertex.glsl", "shaders/lightFragments.glsl");

    if(lightShaderProgram == 0){
        printf("failed to compile lightShaderProgram");
    }

    unsigned int containerTexture = genTexture("textures/container.png", GL_TEXTURE0);
    unsigned int containerSpecular = genTexture("textures/containerSpecular.png", GL_TEXTURE1);

    glBindVertexArray(VAO);

    glEnable(GL_DEPTH_TEST);

    struct Camera camera ={
        .position = {4.0f, 4.0f, 3.0f},
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

    vec3 lightPosition = {5.0f, 2.0f, 2.0f};
    int posDirFlag = 1;
    glUseProgram(shaderProgram);

    float shininess = 64.0f;

    glUniform1i(glGetUniformLocation(shaderProgram, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.specular"), 1);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), shininess);


    vec3 lightAmbient = {0.2f, 0.2f, 0.2f};
    vec3 lightDiffuse = {0.5f, 0.5f, 0.5f};
    vec3 lightSpecular = {1.0f, 1.0f, 1.0f};
    vec3 lightDir = {-0.2f, -1.0f, -0.3f};

    glUniform3fv(glGetUniformLocation(shaderProgram, "light.ambient"), 1, (const float *)lightAmbient);
    glUniform3fv(glGetUniformLocation(shaderProgram, "light.diffuse"), 1, (const float *)lightDiffuse);
    glUniform3fv(glGetUniformLocation(shaderProgram, "light.specular"), 1, (const float *)lightSpecular);
    glUniform3fv(glGetUniformLocation(shaderProgram, "light.direction"), 1, (const float *)lightDir);

    glUniform1f(glGetUniformLocation(shaderProgram, "light.cutOff"), cos(glm_rad(12.5f)));

    glUniform1f(glGetUniformLocation(shaderProgram, "light.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "light.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shaderProgram, "light.quadratic"), 0.032f);

    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, deltaTime);
        glUseProgram(shaderProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerTexture);

        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, (const float *)camera.position);

        vec3 lightColor = {1.0f, 1.0f, 1.0f};
        vec3 objectColor = {0.65f, 0.66, 0.71f};

        glUniform3fv(glGetUniformLocation(shaderProgram, "light.position"), 1, (const float *)lightPosition);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, (const float *)lightPosition);

        glUniform3fv(glGetUniformLocation(shaderProgram, "lightingColor"), 1, (const float *)lightColor);

        glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, (const float *)objectColor);

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
            mat4 normalMatrix;
            glm_mat4_transpose_to(model, normalMatrix);
            glm_mat4_inv(normalMatrix, normalMatrix);
            glm_translate(model, cubePosition.cubePositions[i]);
            //vec3 rotationAxises = {1.0f, -1.0f, 1.0f};
            //glm_rotate(model, (float)glfwGetTime() * glm_rad(50.0f), rotations[i % 10]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (const float *)model);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "normalMatrix"), 1, GL_FALSE, (const float *)normalMatrix);

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

        }

        glUseProgram(lightShaderProgram);
        glUniform3fv(glGetUniformLocation(lightShaderProgram, "lightColor"), 1,  (const float *)lightColor);
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "projection"), 1, GL_FALSE, (const float *)projection);
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "view"), 1, GL_FALSE, (const float *)view);
        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, lightPosition);
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "model"), 1, GL_FALSE, (const float *)model);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();

        lightPosition[0] = sin(currentFrame) * 5;
        lightPosition[1] = fabs(sin(currentFrame) * 5) + 1;
        lightPosition[2] = cos(currentFrame) * 5;
    }
    free(cubePosition.cubePositions);
    glfwTerminate();
    return 0;
}
