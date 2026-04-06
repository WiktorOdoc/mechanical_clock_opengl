#include "Model.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "shaderprogram.h"

bool Model::loadFromFile(const char* filename)
{
    //import model from file throught assimp
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (!scene)
    {
        std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        processMesh(scene->mMeshes[m]);
    }

    return true;
}

void Model::processMesh(aiMesh* mesh)
{
    unsigned int vertexOffset = vertices.size();

    
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        aiVector3D v = mesh->mVertices[i];
        vertices.push_back(glm::vec4(v.x, v.y, v.z, 1.0f));

        aiVector3D n = mesh->mNormals[i];
        normals.push_back(glm::vec4(n.x, n.y, n.z, 0.0f));

        if (mesh->HasTextureCoords(0)) {
            aiVector3D t = mesh->mTextureCoords[0][i];
            texCoords.push_back(glm::vec2(t.x, t.y));
        }
        else {
            texCoords.push_back(glm::vec2(0.0f));
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(vertexOffset + face.mIndices[j]);
        }
    }
}

void Model::setTextures(GLuint tex0, GLuint tex1)
{
    texture0 = tex0;
    texture1 = tex1;
}

void Model::draw(ShaderProgram* sp)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glUniform1i(sp->u("textureMap0"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glUniform1i(sp->u("textureMap1"), 1);

    glUniform1i(sp->u("shaded"), shaded ? 1 : 0); //flaga by Ÿród³o œwiat³a nie by³o ocieniowane

    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, GL_FALSE, 0, vertices.data());

    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, GL_FALSE, 0, normals.data());

    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, GL_FALSE, 0, texCoords.data());


    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());

    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));
}
