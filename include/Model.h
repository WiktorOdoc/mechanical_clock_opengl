#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <GL/glew.h>

class ShaderProgram;

class Model
{
public:
    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<unsigned int> indices;

    bool shaded = true;

    GLuint texture0 = 0;
    GLuint texture1 = 0;

    Model() = default;

    bool loadFromFile(const char* filename);
    void setTextures(GLuint tex0, GLuint tex1);
    void draw(ShaderProgram* sp);
    void setShaded(bool flag) { shaded = flag; }

private:
    void processMesh(aiMesh* mesh);
};

#endif
