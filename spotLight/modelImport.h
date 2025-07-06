#ifndef MODELIMPORT_H
#define MODELIMPORT_H
#include "cglm/cglm.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct Vertex {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
};

struct Texture {
  unsigned int id;
  char *type;
  char *filePath;
};

struct Mesh {
  struct Vertex *vertices;
  unsigned int verticesLen;

  unsigned int *indices;
  unsigned int indicesLen;

  struct Texture *textures;
  unsigned int texturesLen;

  unsigned int VAO, VBO, EBO;
};

struct Model {
  struct Mesh *meshes;
  unsigned int meshesLen;
  struct Texture *texturesLoaded;
  unsigned int texturesLen;
  char *directory;
};

void loadModel(struct Model *model, char *filePath);
void drawModel(struct Model *model, unsigned int shaderId);
void processNode(struct Model *model, struct aiNode *node,
                 const struct aiScene *scene, bool *meshProcessed);
struct Mesh processMesh(struct Model *model, struct aiMesh *mesh,
                        const struct aiScene *scene);
struct Texture *loadMaterialTextures(struct Model *model,
                                     struct aiMaterial *mat,
                                     enum aiTextureType type, char *typeName);

struct Mesh meshConstructor(struct Vertex *vertices, unsigned int verticesLen,
                            unsigned int *indices, unsigned int indicesLen,
                            struct Texture *textures, unsigned int texturesLen);
void drawMesh(struct Mesh *mesh, unsigned int shaderId);
void setupMesh(struct Mesh *mesh);

unsigned int genTexture(char *path, char *directory);

#endif
