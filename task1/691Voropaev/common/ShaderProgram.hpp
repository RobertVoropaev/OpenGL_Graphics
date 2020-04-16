#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

#include <string>
#include <vector>
#include <memory>

#include "Common.h"

/**
Класс для создания и работы с отдельным шейдером
*/
class Shader
{
public:
    Shader(GLenum shaderType) :
        _shaderType(shaderType),
        _id(glCreateShader(shaderType))
    {
    }

    ~Shader()
    {
        glDeleteShader(_id);
    }

    /**
    Читает текст шейдера из файла
    */
    void createFromFile(const std::string& filepath);

    /**
    Использует текст шейдера из строковой переменной
    */
    void createFromString(const std::string &name, const std::string& text);

    /**
    Возвращает идентификатор шейдера
    */
    GLuint id() const { return _id; }

protected:
    Shader(const Shader&) = delete;
    void operator=(const Shader&) = delete;

    GLuint _id;
    GLenum _shaderType;
};

typedef std::shared_ptr<Shader> ShaderPtr;

/**
Класс для работы с шейдерной программой
*/
class ShaderProgram {
public:
    ShaderProgram() :
            _programId(glCreateProgram()) {
    }

    ShaderProgram(const std::string &vertFilepath, const std::string &fragFilepath) :
            _programId(glCreateProgram()) {
        createProgram(vertFilepath, fragFilepath);
    }


    ShaderProgram(const std::string &vertFilepath, const std::string &geomFilePath, const std::string &fragFilepath) :
            _programId(glCreateProgram()) {
        createProgram(vertFilepath, geomFilePath, fragFilepath);
    }

    ~ShaderProgram() {
        glDeleteProgram(_programId);
    }

    void createProgramCompute(const std::string& computeFilePath);

    /**
    Создает шейдерную программу из нескольких шейдеров: вершинного и фрагментного
    */
    void createProgram(const std::string &vertFilepath, const std::string &fragFilepath);

    /**
    Создает шейдерную программу из нескольких шейдеров: вершинного и фрагментного
    */
    void createProgram(const std::string &vertFilepath, const std::string &geomFilePath, const std::string &fragFilepath);

    /**
    Добавляет шейдер к программе
    */
    void attachShader(const ShaderPtr &shader);

    /**
    Линкует программу
    */
    void linkProgram();

    /**
    Возвращает идентификатор программы
    */
    GLuint id() const { return _programId; }

    void use() const {
        glUseProgram(_programId);
    }

    void assertActive() const {
#ifndef NDEBUG
        GLint currentProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        assert(currentProgram == _programId);
#endif
    }

    //----------------------------------------------------------

    void setIntUniform(const std::string &name, const int &value) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniform1i(_programId, uniformLoc, value);
        else {
            assertActive();
            glUniform1i(uniformLoc, value);
        }
    }

    void setFloatUniform(const std::string &name, const float &value) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniform1f(_programId, uniformLoc, value);
        else {
            assertActive();
            glUniform1f(uniformLoc, value);
        }
    }

    void setVec2Uniform(const std::string &name, const glm::vec2 &vec) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniform2fv(_programId, uniformLoc, 1, glm::value_ptr(vec));
        else {
            assertActive();
            glUniform2fv(uniformLoc, 1, glm::value_ptr(vec));
        }
    }

    void setVec3Uniform(const std::string &name, const glm::vec3 &vec) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniform3fv(_programId, uniformLoc, 1, glm::value_ptr(vec));
        else {
            assertActive();
            glUniform3fv(uniformLoc, 1, glm::value_ptr(vec));
        }
    }

    void setVec4Uniform(const std::string &name, const glm::vec4 &vec) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniform4fv(_programId, uniformLoc, 1, glm::value_ptr(vec));
        else {
            assertActive();
            glUniform4fv(uniformLoc, 1, glm::value_ptr(vec));
        }
    }

    void setMat3Uniform(const std::string &name, const glm::mat3 &mat) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniformMatrix3fv(_programId, uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
        else {
            assertActive();
            glUniformMatrix3fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
        }
    }

    void setMat3Uniforms(const std::string &name, const std::vector<glm::mat3> &matrices) {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA) {
            glProgramUniformMatrix3fv(_programId, uniformLoc, matrices.size(), GL_FALSE, reinterpret_cast<const GLfloat *>(matrices.data()));
        }
        else {
            assertActive();
            glUniformMatrix3fv(uniformLoc, matrices.size(), GL_FALSE, reinterpret_cast<const GLfloat*>(matrices.data()));
        }
    }

    void setMat4Uniform(const std::string &name, const glm::mat4 &mat) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniformMatrix4fv(_programId, uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
        else {
            assertActive();
            glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
        }
    }

    void setMat4Uniforms(const std::string &name, const std::vector<glm::mat4> &matrices) {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA) {
            glProgramUniformMatrix4fv(_programId, uniformLoc, matrices.size(), GL_FALSE, reinterpret_cast<const GLfloat *>(matrices.data()));
        }
        else {
            assertActive();
            glUniformMatrix4fv(uniformLoc, matrices.size(), GL_FALSE, reinterpret_cast<const GLfloat*>(matrices.data()));
        }
    }

    void setVec3Uniforms(const std::string &name, const std::vector<glm::vec3> &positions) const {
        GLint uniformLoc = glGetUniformLocation(_programId, name.c_str());
        if (USE_DSA)
            glProgramUniform3fv(_programId, uniformLoc, static_cast<GLsizei>(positions.size()), reinterpret_cast<const GLfloat *>(positions.data()));
        else {
            assertActive();
            glUniform3fv(uniformLoc, static_cast<GLsizei>(positions.size()), reinterpret_cast<const GLfloat *>(positions.data()));
        }
    }

protected:
    ShaderProgram(const ShaderProgram &) = delete;
    void operator=(const ShaderProgram &) = delete;

    GLuint _programId;

    std::vector<ShaderPtr> _shaders;
};

typedef std::shared_ptr<ShaderProgram> ShaderProgramPtr;
