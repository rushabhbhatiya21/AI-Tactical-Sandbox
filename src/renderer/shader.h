#pragma once

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <glm/glm.hpp>
#include <string>


class Shader
{
public:
    // the program ID
    GLuint id = 0;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // use/activate the shader
    void use() const;

    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void set4Float(const std::string& name, float v0, float v1, float v2, float v3) const;
    void setMat4(const std::string& name, const glm::mat4& matrix4) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec3(const std::string& name, const glm::vec3& vec3) const;
};