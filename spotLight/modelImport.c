#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "modelImport.h"


unsigned int genTexture(char *path, char *directory){

    char fullPath[1024];
    //the path is the textures path relative to the model and then the directory is the absolute directory to the model
    sprintf(fullPath, "%s/%s", directory, path);

    stbi_set_flip_vertically_on_load(true);

    unsigned int texture;

    int width, height, nChannels;
    unsigned char *data = stbi_load(fullPath, &width, &height, &nChannels, 0);
    if(data == NULL){
        printf("ERROR: Failed to laod texture %s\n", fullPath);
        return 0;
    }

    glGenTextures(1, &texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format = GL_RGB;
    if(nChannels == 1) format = GL_RED;
    else if(nChannels == 4) format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
}



unsigned int totalIndices(struct aiMesh *mesh){
    unsigned int totalIndices = 0;
    for(int i = 0; i < mesh->mNumFaces; i++){
        struct aiFace face = mesh->mFaces[i];
        totalIndices += face.mNumIndices;
    }
    return totalIndices;
}


struct Texture* loadMaterialTextures(struct Model *model, struct aiMaterial *mat, enum aiTextureType type, char *typeName){
    unsigned int amountOfTextures = aiGetMaterialTextureCount(mat, type);
    struct Texture *textures = malloc(sizeof(struct Texture) * amountOfTextures);

    for(int i = 0; i < amountOfTextures; i++){
        struct aiString str;
        aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);
        //This should probably be a hashmap, but this is how the book does it
        bool skip = false;
        for(int j = 0; j < model->texturesLen; j++){
            if(strcmp(str.data, model->texturesLoaded[j].filePath) == 0){
                textures[i] = model->texturesLoaded[j];
                skip = true;
                break;
            }
        }
        if(!skip){
            textures[i].id = genTexture(str.data, model->directory);
            textures[i].type = malloc(sizeof(char) * (strlen(typeName) + 1));
            sprintf(textures[i].type, "%s", typeName);
            textures[i].filePath = malloc(sizeof(char) * (strlen(str.data) + 1));
            sprintf(textures[i].filePath, "%s", str.data);
            model->texturesLoaded[model->texturesLen++] = textures[i];
        }
    }

    return textures;

}


struct Mesh processMesh(struct Model *model, struct aiMesh *mesh, const struct aiScene *scene){
    struct Mesh myMesh;
    myMesh.vertices = malloc(sizeof(struct Vertex) * mesh->mNumVertices);
    myMesh.verticesLen = mesh->mNumVertices;

    for(int i = 0; i < mesh->mNumVertices; i++){
        myMesh.vertices[i].position[0] = mesh->mVertices[i].x;
        myMesh.vertices[i].position[1] = mesh->mVertices[i].y;
        myMesh.vertices[i].position[2] = mesh->mVertices[i].z;

        myMesh.vertices[i].normal[0] = mesh->mNormals[i].x;
        myMesh.vertices[i].normal[1] = mesh->mNormals[i].y;
        myMesh.vertices[i].normal[2] = mesh->mNormals[i].z;

        if(mesh->mTextureCoords[0]){    //some meshes do not contain texcoords
            myMesh.vertices[i].texCoords[0] = mesh->mTextureCoords[0][i].x;
            myMesh.vertices[i].texCoords[1] = mesh->mTextureCoords[0][i].y;
        }
        else{
            myMesh.vertices[i].texCoords[0] = 0.0f;
            myMesh.vertices[i].texCoords[1] = 0.0f;
        }
    }

    myMesh.indicesLen = totalIndices(mesh);
    myMesh.indices = malloc(sizeof(unsigned int) * myMesh.indicesLen);

    unsigned int index = 0;
    for(int i = 0; i < mesh->mNumFaces; i++){
        struct aiFace face = mesh->mFaces[i];
        for(int j = 0; j < face.mNumIndices; j++){
            myMesh.indices[index++] = face.mIndices[j];
        }
    }


    if(mesh->mMaterialIndex >= 0){
        struct aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
        unsigned int diffuseCount = aiGetMaterialTextureCount(mat, aiTextureType_DIFFUSE);
        unsigned int specularCount = aiGetMaterialTextureCount(mat, aiTextureType_SPECULAR);
        myMesh.texturesLen = diffuseCount + specularCount;
        myMesh.textures = malloc(sizeof(struct Texture) * myMesh.texturesLen);
        struct Texture *diffuseMaps = loadMaterialTextures(model, mat, aiTextureType_DIFFUSE, "texture_diffuse");
        struct Texture *specularMaps = loadMaterialTextures(model, mat, aiTextureType_SPECULAR, "texture_specular");
        for(int i = 0; i < diffuseCount; i++){
            myMesh.textures[i] = diffuseMaps[i];
        }
        for(int i = 0; i < specularCount; i++){
            myMesh.textures[diffuseCount + i] = specularMaps[i];
        }

    }
    else{
        myMesh.texturesLen = 0;
    }

    setupMesh(&myMesh);

    return myMesh;
}


void processNode(struct Model *model, struct aiNode *node, const struct aiScene *scene, bool *meshProcessed){
    for(int i = 0; i < node->mNumMeshes; i++){
        if(meshProcessed[node->mMeshes[i]] != true){
            model->meshes[node->mMeshes[i]] = processMesh(model, scene->mMeshes[node->mMeshes[i]], scene);
            meshProcessed[node->mMeshes[i]] = true;
        }
    }
    for(int i = 0; i < node->mNumChildren; i++){
        processNode(model, node->mChildren[i], scene, meshProcessed);
    }
}


void loadModel(struct Model *model, char *filePath){
    const struct aiScene *scene = aiImportFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(scene == NULL || scene->mRootNode == NULL || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)){
        printf("Error loading Model: %s\n", filePath);
        return;
    }

    bool *meshProcessed = calloc(scene->mNumMeshes, sizeof(bool));
    model->meshes = malloc(sizeof(struct Mesh) * scene->mNumMeshes);
    model->meshesLen = scene->mNumMeshes;
    model->texturesLoaded = malloc(sizeof(struct Texture) * 128);
    model->texturesLen = 0;

    char *lastSlash = strrchr(filePath, '/');
    if (lastSlash) {
        size_t dirLen = lastSlash - filePath;
        model->directory = malloc(sizeof(char) * (dirLen + 1));
        strncpy(model->directory, filePath, dirLen);
        model->directory[dirLen] = '\0';
    }

    processNode(model, scene->mRootNode, scene, meshProcessed);

    printf("model->textureLen: %d\n",model->texturesLen);
    free(meshProcessed);
}


void drawModel(struct Model *model, unsigned int shaderId){
    int i = 0;
    mat4 modelMatrix;
    glm_mat4_identity(modelMatrix);
    glm_translate(modelMatrix, (vec3){0.0f, 0.0f, 0.0f});
    glm_scale(modelMatrix, (vec3){10.0f, 10.0f, 10.0f});  // try increasing this

    glUniformMatrix4fv(glGetUniformLocation(shaderId, "model"), 1, GL_FALSE, (const float *)modelMatrix);

    while(i < model->meshesLen){
        drawMesh(&model->meshes[i++], shaderId);
    }
}


void drawMesh(struct Mesh *mesh, unsigned int shaderId){
    char name[128];
    int diffuseAmount = 0;
    int specularAmount = 0;

    for(int i = 0; i < mesh->texturesLen; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        if(strcmp(mesh->textures[i].type, "texture_diffuse") == 0){
            sprintf(name, "material.diffuse%d", diffuseAmount);
            diffuseAmount++;
        }
        else{
            sprintf(name, "material.specular%d", specularAmount);
            specularAmount++;
        }

        glUniform1i(glGetUniformLocation(shaderId, "diffuseCount"), diffuseAmount);
        glUniform1i(glGetUniformLocation(shaderId, "specularCount"), specularAmount);

        glUniform1i(glGetUniformLocation(shaderId, name), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
        //printf("%s\n", name);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(mesh->VAO);
    printf("Drawing mesh with %d indices and %d textures\n", mesh->indicesLen, mesh->texturesLen);
    glDrawElements(GL_TRIANGLES, mesh->indicesLen, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void setupMesh(struct Mesh *mesh){
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

    glBufferData(GL_ARRAY_BUFFER, mesh->verticesLen * sizeof(struct Vertex), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesLen * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);

    //vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*) 0);

    //vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(3 * sizeof(float)));

    //vertex texcoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

struct Mesh meshConstructor(struct Vertex *vertices, unsigned int verticesLen, unsigned int *indices, unsigned int indicesLen, struct Texture *textures, unsigned int texturesLen){
    struct Mesh mesh;
    mesh.vertices = vertices;
    mesh.verticesLen = verticesLen;

    mesh.indices = indices;
    mesh.indicesLen = indicesLen;

    mesh.textures = textures;
    mesh.texturesLen = texturesLen;

    setupMesh(&mesh);

    return mesh;
}
