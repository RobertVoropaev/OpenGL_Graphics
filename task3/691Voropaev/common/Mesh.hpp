#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Common.h"

/**
Абстракция буфера с данными в видеопамяти
*/
class DataBuffer
{
public:
    /**
    Создает буфер с данными
    \param target тип буфера (GL_ARRAY_BUFFER, GL_TEXTURE_BUFFER и другие)
    */
    DataBuffer(GLenum target = GL_ARRAY_BUFFER):
        _target(target)
    {
	    if (USE_DSA) {
		    glCreateBuffers(1, &_vbo);
	    }
	    else {
		    glGenBuffers(1, &_vbo);
	    }
    }

    ~DataBuffer()
    {
        glDeleteBuffers(1, &_vbo);
    }

    /**
    Копирует данные из оперативной памяти в видеопамять, выделяя память под данные при необходимости
    \param size размер данных в байтах
    \param data указатель на начало массива данных в оперативной памяти
    */
    void setData(GLsizeiptr size, const GLvoid* data)
    {
        glBindBuffer(_target, _vbo);
        glBufferData(_target, size, data, GL_STATIC_DRAW);
        glBindBuffer(_target, 0);
    }

    void initStorage(GLsizeiptr size, const GLvoid* data, GLbitfield flags) {
        assert(USE_DSA);
        glNamedBufferStorage(_vbo, size, data, flags);
    }

    void bind() const
    {
        glBindBuffer(_target, _vbo);
    }

    void unbind() const
    {
        glBindBuffer(_target, 0);
    }

    /**
    Возвращает идентификатор шейдера
    */
    GLuint id() const { return _vbo; }

protected:
    DataBuffer(const DataBuffer&) = delete;
    void operator=(const DataBuffer&) = delete;

    ///Идентификатор буфера
    GLuint _vbo;

    ///Тип буфера
    GLenum _target;
};

typedef std::shared_ptr<DataBuffer> DataBufferPtr;

/**
Абстракция полигональной модели
Инкапсулирует:
- управляющий объект VertexArrayObject
- буферы с вершинными атрибутами VertexBufferObject (один буфер может содержать данные для нескольких атрибутов)
- тип рисуемых примитивов
- количество вершин в модели
- матрицу модели (local to world)
*/
class Mesh
{
public:
    Mesh() :
        _primitiveType(GL_TRIANGLES),
        _vertexCount(0)
    {
        glGenVertexArrays(1, &_vao);
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &_vao);
    }
    
    /**
    Устанавливает параметры вершинного атрибута полигональной модели
    \param index номер атрибута (0, 1, 2, ...)
    \param size количество компонентов в атрибуте (1, 2, 3 или 4)
    \param type тип данных компонентов атрибута (GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_FIXED и другие)
    \param normalized должны ли данные быть нормализованы на интервал 0..1 (GL_TRUE) или использоваться как есть (GL_FALSE)
    \param stride расстояние в байтах между атрибутами двух последовательных вершин. Если 0, то атрибуты расположены в буфере плотно (без промежутков)
    \param offset сдвиг в байтах от начала буфера
    \param buffer буфер с данными, где хранятся значения атрибута
    */
    void setAttribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint offset, const DataBufferPtr& buffer)
    {
        _buffers[index] = buffer; //чтобы буфер не был удален раньше модели

        glBindVertexArray(_vao);

        buffer->bind();
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, reinterpret_cast<void*>(offset));
        buffer->unbind();

        glBindVertexArray(0);
    }

    /**
     * Устанавливает параметры вершинного атрибута полигональной модели.
     * Версия для signed/unsigned byte/short/int типов.
     */
    void setAttributeI(GLuint index, GLint size, GLenum type, GLsizei stride, GLuint offset, const DataBufferPtr& buffer)
    {
        _buffers[index] = buffer; //чтобы буфер не был удален раньше модели

        glBindVertexArray(_vao);

        buffer->bind();
        glEnableVertexAttribArray(index);
        glVertexAttribIPointer(index, size, type, stride, reinterpret_cast<void*>(offset));
        buffer->unbind();

        glBindVertexArray(0);
    }

    /**
    Устанавливает разделитель для вершинного атрибута (разбирается на 10м семинаре, используется при инстансинге)
    \param index номер атрибута (0, 1, 2, ...)
    \param divisor если 0, то одно значение атрибута соответствует одной вершине, если >0, то одно значение атрибута соответствует всем вершинам для divisor инстансов модели
    */
    void setAttributeDivisor(GLuint index, GLuint divisor)
    {
        glBindVertexArray(_vao);

        glVertexAttribDivisor(index, divisor);

        glBindVertexArray(0);
    }

    /**
    Устанавливает тип примитива (GL_POINTS, GL_LINES, GL_TRIANGLES и другие)
    */
    void setPrimitiveType(GLuint type) { _primitiveType = type; }

    /**
    Устанавливает количество вершин, которые должны быть отрендерены
    */
    void setVertexCount(GLuint count) { _vertexCount = count; }

    void setIndices(GLuint indicesCount, const DataBufferPtr &indexBuffer) {
        _hasIndices = true;
        _indicesCount = indicesCount;
        _indexBuffer = indexBuffer;

        glBindVertexArray(_vao);
        indexBuffer->bind();
        glBindVertexArray(0);
    }

    /**
    Матрица модели (преобразует локальные координаты в мировые)
    */
    glm::mat4 modelMatrix() const { return _modelMatrix; }

    /**
    Устанавливает матрицу модели
    */
    void setModelMatrix(const glm::mat4& m) { _modelMatrix = m; }

    GLuint getTrianglesCount() const {
        if (_hasIndices)
            return _indicesCount / 3;
        else
            return _vertexCount / 3;
    }

    /**
    Рисует модель
    */
    void draw() const
    {
        glBindVertexArray(_vao);
        if (_hasIndices) {
            glDrawElements(_primitiveType, _indicesCount, GL_UNSIGNED_INT, nullptr);
        }
        else {
            glDrawArrays(_primitiveType, 0, _vertexCount);
        }
    }

    void drawArrays(GLint first, GLsizei count) {
        assert(!_hasIndices);
        glBindVertexArray(_vao);
        glDrawArrays(_primitiveType, first, count);
    }

    /**
     * Запускает multiDraw по одним и тем же смещениям.
     * Ненатурально, используется для демонстрации.
     * Случай с несколькими мэшами в одном буфере потребует задания смещений.
     */
    void multiDraw(GLsizei drawCount) {
        glBindVertexArray(_vao);
        if (_hasIndices) {
            std::vector<GLsizei> counts(drawCount, _indicesCount);
            std::vector<const GLvoid*> offsets(drawCount, 0);
            glMultiDrawElements(_primitiveType, counts.data(), GL_UNSIGNED_INT, offsets.data(), drawCount);
        }
        else {
            std::vector<GLint> offsets(drawCount, 0);
            std::vector<GLsizei> counts(drawCount, _vertexCount);
            glMultiDrawArrays(_primitiveType, offsets.data(), counts.data(), drawCount);
        }
    }

	void multiDrawArrays(GLsizei drawCount, const std::vector<GLint> &offsets, const std::vector<GLsizei> &counts) {
		assert(!_hasIndices);
		glMultiDrawArrays(_primitiveType, offsets.data(), counts.data(), drawCount);
	}

    /**
    Рисует модель instanceCount раз (разбирается на 10м семинаре)
    */
    void drawInstanced(unsigned int instanceCount) const
    {
        glBindVertexArray(_vao);
        if (_hasIndices) {
            glDrawElementsInstanced(_primitiveType, _indicesCount, GL_UNSIGNED_INT, nullptr, instanceCount);
        }
        else {
            glDrawArraysInstanced(_primitiveType, 0, _vertexCount, instanceCount);
        }
    }

	GLsizei getVertexCount() const { return _vertexCount; }

    GLuint getVAO() const { return _vao; }

protected:
    Mesh(const Mesh&) = delete;
    void operator=(const Mesh&) = delete;

    ///Идентификатор Vertex Array Object
    GLuint _vao;

    ///Буферы с данными - храним здесь, чтобы они не были удалены раньше модели
    std::map<GLuint, DataBufferPtr> _buffers;

    DataBufferPtr _indexBuffer;

    ///Тип геометрического примитива
    GLuint _primitiveType;

    ///Количество вершин в модели
    GLuint _vertexCount;

    bool _hasIndices = false;

    GLuint _indicesCount = 0;

    ///Матрица модели (local to world)
    glm::mat4 _modelMatrix;
};

typedef std::shared_ptr<Mesh> MeshPtr;

//=========== Функции для создания тестовых мешей

/**
Создает модель сферы
*/
MeshPtr makeSphere(float radius, unsigned int N = 100);

/**
Создает модель куба размером 2 * size
*/
MeshPtr makeCube(float size);

/**
Создает прямоугольник (2 треугольника), заполняющий экран (координаты в Clip Space)
*/
MeshPtr makeScreenAlignedQuad();

/**
Создает плоскость земли размером от -size до +size по осям XY
Генерирует текстурные координаты, так чтобы на плоскости размещалось 2 * numTiles по каждой оси
*/
MeshPtr makeGroundPlane(float size, float numTiles);

class aiMesh;
MeshPtr loadFromAIMesh(const aiMesh &sourceMesh);

/**
Загружает меш из внешнего файла с помощью библиотеки Assimp
*/
MeshPtr loadFromFile(const std::string& filename, int meshIndex = 0);