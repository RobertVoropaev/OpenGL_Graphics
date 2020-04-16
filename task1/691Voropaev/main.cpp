#include "common/Application.hpp"
#include "common/Mesh.hpp"
#include "common/ShaderProgram.hpp"

#include <iostream>
#include <vector>

MeshPtr makeCeil(float size)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    //triangle 1
    vertices.push_back(glm::vec3(-size, size, 0.0f));
    vertices.push_back(glm::vec3(size, size, 0.0f));
    vertices.push_back(glm::vec3(size, -size, 0.0f));

    normals.push_back(glm::vec3(0.0, 0.0, 1.0));
    normals.push_back(glm::vec3(0.0, 0.0, 1.0));
    normals.push_back(glm::vec3(0.0, 0.0, 1.0));

    //triangle 2
    vertices.push_back(glm::vec3(-size, size, 0.0f));
    vertices.push_back(glm::vec3(size, -size, 0.0f));
    vertices.push_back(glm::vec3(-size, -size, 0.0f));

    normals.push_back(glm::vec3(0.0, 0.0, 1.0));
    normals.push_back(glm::vec3(0.0, 0.0, 1.0));
    normals.push_back(glm::vec3(0.0, 0.0, 1.0));

    //----------------------------------------

    DataBufferPtr buf0 = std::make_shared<DataBuffer>(GL_ARRAY_BUFFER);
    buf0->setData(vertices.size() * sizeof(float) * 3, vertices.data());

    DataBufferPtr buf1 = std::make_shared<DataBuffer>(GL_ARRAY_BUFFER);
    buf1->setData(normals.size() * sizeof(float) * 3, normals.data());


    MeshPtr mesh = std::make_shared<Mesh>();
    mesh->setAttribute(0, 3, GL_FLOAT, GL_FALSE, 0, 0, buf0);
    mesh->setAttribute(1, 3, GL_FLOAT, GL_FALSE, 0, 0, buf1);
    mesh->setPrimitiveType(GL_TRIANGLES);
    mesh->setVertexCount(vertices.size());

    std::cout << "Ceiling is created";

    return mesh;
}

class Task1 : public Application
{
public:
    MeshPtr _ceil;
    MeshPtr _labyrinth;

    ShaderProgramPtr _shader;

    GLuint _ubo;
    GLuint uniformBlockBinding = 0;

    void makeScene() override
    {
        Application::makeScene();

        _ceil = makeCeil(10.0);
        glm::mat4 mat_ceil(1.0f);
        mat_ceil = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.90f));
        _ceil->setModelMatrix(mat_ceil);

        _labyrinth = loadFromFile("691VoropaevData1/models/labyrinth.obj");
        glm::mat4 mat_lab(1.0f);
        //mat_lab = glm::translate(mat_lab, glm::vec3(0.0f, 0.0f, 0.0f));
        mat_lab = glm::rotate(mat_lab, 3.14f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
        _labyrinth->setModelMatrix(mat_lab);

        //=========================================================
        //Инициализация шейдеров

        _shader = std::make_shared<ShaderProgram>("691VoropaevData1/shader.vert", "691VoropaevData1/shader.frag");

        //=========================================================
        //Инициализация Uniform Buffer Object

        // Выведем размер Uniform block'а.
        GLint uniformBlockDataSize;
        glGetActiveUniformBlockiv(_shader->id(), 0, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockDataSize);
        std::cout << "Uniform block 0 data size = " << uniformBlockDataSize << std::endl;

        if (USE_DSA) {
            glCreateBuffers(1, &_ubo);
            glNamedBufferData(_ubo, uniformBlockDataSize, nullptr, GL_DYNAMIC_DRAW);
        }
        else {
            glGenBuffers(1, &_ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
            glBufferData(GL_UNIFORM_BUFFER, uniformBlockDataSize, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        // Привязываем буффер к точке привязки Uniform буферов.
        glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, _ubo);


        // Получение информации обо всех uniform-переменных шейдерной программы.
        if (USE_INTERFACE_QUERY) {
            GLsizei uniformsCount;
            GLsizei maxNameLength;
            glGetProgramInterfaceiv(_shader->id(), GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformsCount);
            glGetProgramInterfaceiv(_shader->id(), GL_UNIFORM, GL_MAX_NAME_LENGTH, &maxNameLength);
            std::vector<char> nameBuffer(maxNameLength);

            std::vector<GLenum> properties = {GL_TYPE, GL_ARRAY_SIZE, GL_OFFSET, GL_BLOCK_INDEX};
            enum Property {
                Type,
                ArraySize,
                Offset,
                BlockIndex
            };
            for (GLuint uniformIndex = 0; uniformIndex < uniformsCount; uniformIndex++) {
                std::vector<GLint> params(properties.size());
                glGetProgramResourceiv(_shader->id(), GL_UNIFORM, uniformIndex, properties.size(), properties.data(),
                                       params.size(), nullptr, params.data());
                GLsizei realNameLength;
                glGetProgramResourceName(_shader->id(), GL_UNIFORM, uniformIndex, maxNameLength, &realNameLength, nameBuffer.data());

                std::string uniformName = std::string(nameBuffer.data(), realNameLength);

                std::cout << "Uniform " << "index = " << uniformIndex << ", name = " << uniformName << ", block = " << params[BlockIndex] << ", offset = " << params[Offset] << ", array size = " << params[ArraySize] << ", type = " << params[Type] << std::endl;
            }
        }
        else {
            GLsizei uniformsCount;
            glGetProgramiv(_shader->id(), GL_ACTIVE_UNIFORMS, &uniformsCount);

            std::vector<GLuint> uniformIndices(uniformsCount);
            for (int i = 0; i < uniformsCount; i++)
                uniformIndices[i] = i;
            std::vector<GLint> uniformBlocks(uniformsCount);
            std::vector<GLint> uniformNameLengths(uniformsCount);
            std::vector<GLint> uniformTypes(uniformsCount);
            glGetActiveUniformsiv(_shader->id(), uniformsCount, uniformIndices.data(), GL_UNIFORM_BLOCK_INDEX, uniformBlocks.data());
            glGetActiveUniformsiv(_shader->id(), uniformsCount, uniformIndices.data(), GL_UNIFORM_NAME_LENGTH, uniformNameLengths.data());
            glGetActiveUniformsiv(_shader->id(), uniformsCount, uniformIndices.data(), GL_UNIFORM_TYPE, uniformTypes.data());

            for (int i = 0; i < uniformsCount; i++) {
                std::vector<char> name(uniformNameLengths[i]);
                GLsizei writtenLength;
                glGetActiveUniformName(_shader->id(), uniformIndices[i], name.size(), &writtenLength, name.data());
                std::string uniformName = name.data();

                std::cout << "Uniform " << "index = " << uniformIndices[i] << ", name = " << uniformName << ", block = " << uniformBlocks[i] << ", type = " << uniformTypes[i] << std::endl;
            }
        }
    }

    void update() override
    {
        Application::update();

        //Обновляем содержимое Uniform Buffer Object

        //Вариант для буферов, у которых layout отличается от std140

        //Имена юниформ-переменных
        const char* names[2] =
                {
                        "viewMatrix",
                        "projectionMatrix"
                };

        GLuint index[2];
        GLint offset[2];

        //Запрашиваем индексы 2х юниформ-переменных
        glGetUniformIndices(_shader->id(), 2, names, index);

        //Зная индексы, запрашиваем сдвиги для 2х юниформ-переменных
        glGetActiveUniformsiv(_shader->id(), 2, index, GL_UNIFORM_OFFSET, offset);

        // Вывод оффсетов.
        static bool hasOutputOffset = false;
        if (!hasOutputOffset) {
            std::cout << "Offsets: viewMatrix " << offset[0] << ", projMatrix " << offset[1] << std::endl;
            hasOutputOffset = true;
        }

        //Устанавливаем значения 2х юниформ-перменных по отдельности
        if (USE_DSA) {
            glNamedBufferSubData(_ubo, offset[0], sizeof(_camera->viewMatrix), &_camera->viewMatrix);
            glNamedBufferSubData(_ubo, offset[1], sizeof(_camera->projMatrix), &_camera->projMatrix);
        }
        else {
            glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, offset[0], sizeof(_camera->viewMatrix), &_camera->viewMatrix);
            glBufferSubData(GL_UNIFORM_BUFFER, offset[1], sizeof(_camera->projMatrix), &_camera->projMatrix);
        }
    }

    void draw() override
    {
        Application::draw();

        //Получаем текущие размеры экрана и выставлям вьюпорт
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        glViewport(0, 0, width, height);

        //Очищаем буферы цвета и глубины от результатов рендеринга предыдущего кадра
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Подключаем шейдер
        _shader->use();

        //Загружаем на видеокарту значения юниформ-переменных
        unsigned int blockIndex = glGetUniformBlockIndex(_shader->id(), "Matrices");
        glUniformBlockBinding(_shader->id(), blockIndex, uniformBlockBinding);

        //Загружаем на видеокарту матрицы модели мешей и запускаем отрисовку
        _shader->setMat4Uniform("modelMatrix", _ceil->modelMatrix());
        _ceil->draw();

        _shader->setMat4Uniform("modelMatrix", _labyrinth->modelMatrix());
        _labyrinth->draw();
    }
};

int main() {
    Task1 app;
    app.start();

    return 0;
}