#include "common/Application.hpp"
#include "common/LightInfo.hpp"
#include "common/Mesh.hpp"
#include "common/ShaderProgram.hpp"
#include "common/Texture.hpp"

#include <iostream>

GLuint loadBMP_custom(const char * imagepath){

    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file){
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        getchar();
        return 0;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 ){
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    // Make sure this is a 24bpp file
    if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}
    if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Create a buffer
    data = new unsigned char [imageSize];

    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);

    // Everything is in memory now, the file can be closed.
    fclose (file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete [] data;

    // Poor filtering, or ...
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // ... which requires mipmaps. Generate them automatically.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Return the ID of the texture we just created
    return textureID;
}


class SampleApplication : public Application
{
public:
    MeshPtr _lab;

    MeshPtr _cube;
    MeshPtr _sphere;
    MeshPtr _bunny;

    MeshPtr _quad;

    MeshPtr _marker; //Меш - маркер для источника света

    //Идентификатор шейдерной программы
    ShaderProgramPtr _markerShader;
    ShaderProgramPtr _renderToShadowMapShader;
    ShaderProgramPtr _commonWithShadowsShader;
    ShaderProgramPtr _commonWithShadowsShader_usualSampler;
    ShaderProgramPtr _commonWithShadowsShaderVar2;

    float depthDebugPower = 50.0f;

    //Переменные для управления положением одного источника света
    float _lr = 7.6f;
    float _phi = 0.58f;
    float _theta = 0.48f;

    LightInfo _light;
    CameraInfo _lightCamera;

    TexturePtr _labTex;
    TexturePtr _objTex;

    GLuint _sampler;
    GLuint _depthSampler;
    GLuint _depthSamplerLinear;
    GLuint _depthSampler_nocomparison;
    GLuint _depthSamplerLinear_nocomparison;

    GLuint _normalTexture;

    GLuint _framebufferId;
    GLuint _depthTexId;
    unsigned int _fbWidth = 1024;
    unsigned int _fbHeight = 1024;

    bool _isLinearSampler = false;
    bool _cullFrontFaces = true;
    bool _randomPoints = true;
    bool _nocomparison = false;
    bool _usesmoothstep = false;
    bool _useBias = true;
    float _bias = 0.0f;

    void initFramebuffer()
    {
        //Создаем фреймбуфер
        glGenFramebuffers(1, &_framebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);

        //----------------------------

        //Создаем текстуру, куда будем впоследствии копировать буфер глубины
        glGenTextures(1, &_depthTexId);
        glBindTexture(GL_TEXTURE_2D, _depthTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, _fbWidth, _fbHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTexId, 0);

        //----------------------------

        // Указываем, что для текущего фреймбуфера первый выход фрагментного шейдера никуда не пойдет.
        GLenum buffers[] = { GL_NONE };
        glDrawBuffers(1, buffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Failed to setup framebuffer\n";
            exit(1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void makeScene() override
    {
        Application::makeScene();

        //=========================================================
        //Создание и загрузка мешей
        {
            _lab = loadFromFile("691VoropaevData2/models/new_labyrinth.obj");
            glm::mat4 mat_lab(1.0f);
            mat_lab = glm::rotate(mat_lab, 3.14f / 2, glm::vec3(1.0f, 0.0f, 0.0f));
            _lab->setModelMatrix(mat_lab);

        }

        {
            _cube = makeCube(0.5f);
            _cube->setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.15f)) *
                                  glm::scale(glm::mat4(1.0f), glm::vec3(0.3f)));

            _sphere = makeSphere(0.5f);
            _sphere->setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.15f)) *
                                    glm::scale(glm::mat4(1.0f), glm::vec3(0.3f)));

            _bunny = loadFromFile("691VoropaevData2/models/bunny.obj");
            _bunny->setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) *
                                   glm::scale(glm::mat4(1.0f), glm::vec3(0.3f)));
        }

        _marker = makeSphere(0.1f);

        _quad = makeScreenAlignedQuad();

        //=========================================================
        //Инициализация шейдеров

        _markerShader = std::make_shared<ShaderProgram>("691VoropaevData2/shaders/marker.vert", "691VoropaevData2/shaders/marker.frag");
        _renderToShadowMapShader = std::make_shared<ShaderProgram>("691VoropaevData2/shaders/toshadow.vert", "691VoropaevData2/shaders/toshadow.frag");
        _commonWithShadowsShader = std::make_shared<ShaderProgram>("691VoropaevData2/shaders/shadow.vert", "691VoropaevData2/shaders/shadow.frag");
        _commonWithShadowsShader_usualSampler = std::make_shared<ShaderProgram>("691VoropaevData2/shaders/shadow.vert", "691VoropaevData2/shaders/shadow_usualsampler.frag");
        _commonWithShadowsShaderVar2 = std::make_shared<ShaderProgram>("691VoropaevData2/shaders/shadow.vert", "691VoropaevData2/shaders/shadow2.frag");

        //=========================================================
        //Инициализация значений переменных освщения
        _light.position = glm::vec3(glm::cos(_phi) * glm::cos(_theta), glm::sin(_phi) * glm::cos(_theta), glm::sin(_theta)) * (float)_lr;
        _light.ambient = glm::vec3(0.2, 0.2, 0.2);
        _light.diffuse = glm::vec3(0.8, 0.8, 0.8);
        _light.specular = glm::vec3(1.0, 1.0, 1.0);

        //=========================================================
        //Загрузка и создание текстур
        _labTex = loadTexture("691VoropaevData2/images/brick.jpg");
        _objTex = loadTexture("691VoropaevData2/images/earth_global.jpg");

        _normalTexture = loadBMP_custom("691VoropaevData2/images/normal.bmp");

        //=========================================================
        //Инициализация сэмплера, объекта, который хранит параметры чтения из текстуры
        glGenSamplers(1, &_sampler);
        glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLfloat border[] = { 1.0f, 0.0f, 0.0f, 1.0f };

        glGenSamplers(1, &_depthSampler);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(_depthSampler, GL_TEXTURE_BORDER_COLOR, border);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glSamplerParameteri(_depthSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

        glGenSamplers(1, &_depthSamplerLinear);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(_depthSamplerLinear, GL_TEXTURE_BORDER_COLOR, border);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glSamplerParameteri(_depthSamplerLinear, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

        glGenSamplers(1, &_depthSampler_nocomparison);
        glSamplerParameteri(_depthSampler_nocomparison, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(_depthSampler_nocomparison, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(_depthSampler_nocomparison, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(_depthSampler_nocomparison, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(_depthSampler_nocomparison, GL_TEXTURE_BORDER_COLOR, border);

        glGenSamplers(1, &_depthSamplerLinear_nocomparison);
        glSamplerParameteri(_depthSamplerLinear_nocomparison, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(_depthSamplerLinear_nocomparison, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(_depthSamplerLinear_nocomparison, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(_depthSamplerLinear_nocomparison, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(_depthSamplerLinear_nocomparison, GL_TEXTURE_BORDER_COLOR, border);


        //=========================================================
        //Инициализация фреймбуфера для рендера теневой карты

        initFramebuffer();
    }



    void update()
    {
        Application::update();

        _light.position = glm::vec3(glm::cos(_phi) * glm::cos(_theta), glm::sin(_phi) * glm::cos(_theta), glm::sin(_theta)) * _lr;
        _lightCamera.viewMatrix = glm::lookAt(_light.position, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        _lightCamera.projMatrix = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 130.f);
    }

    void draw() override
    {
        drawToShadowMap(_lightCamera);
        drawToScreen(_randomPoints ?
                     _commonWithShadowsShaderVar2 :
                     (_nocomparison ?
                      _commonWithShadowsShader_usualSampler :
                      _commonWithShadowsShader),
                     _camera, _lightCamera);
    }

    void drawToShadowMap(const CameraInfo& lightCamera)
    {
        //=========== Сначала подключаем фреймбуфер и рендерим в текстуру ==========
        glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId);

        glViewport(0, 0, _fbWidth, _fbHeight);
        glClear(GL_DEPTH_BUFFER_BIT);

        _renderToShadowMapShader->use();
        _renderToShadowMapShader->setMat4Uniform("lightViewMatrix", lightCamera.viewMatrix);
        _renderToShadowMapShader->setMat4Uniform("lightProjectionMatrix", lightCamera.projMatrix);

        if (_cullFrontFaces)
        {
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glCullFace(GL_FRONT);
        }

        drawLab(_renderToShadowMapShader, lightCamera);
        drawObj(_renderToShadowMapShader, lightCamera);

        if (_cullFrontFaces)
        {
            glDisable(GL_CULL_FACE);
        }

        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //Отключаем фреймбуфер
    }

    void drawToScreen(const ShaderProgramPtr& shader, const CameraInfo& camera, const CameraInfo& lightCamera)
    {
        //Получаем текущие размеры экрана и выставлям вьюпорт
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        glViewport(0, 0, width, height);

        //Очищаем буферы цвета и глубины от результатов рендеринга предыдущего кадра
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->use();
        shader->setMat4Uniform("viewMatrix", camera.viewMatrix);
        shader->setMat4Uniform("projectionMatrix", camera.projMatrix);

        glm::vec3 lightPosCamSpace = glm::vec3(camera.viewMatrix * glm::vec4(_light.position, 1.0));

        shader->setVec3Uniform("light.pos", lightPosCamSpace); //копируем положение уже в системе виртуальной камеры
        shader->setVec3Uniform("light.La", _light.ambient);
        shader->setVec3Uniform("light.Ld", _light.diffuse);
        shader->setVec3Uniform("light.Ls", _light.specular);

        {
            shader->setMat4Uniform("lightViewMatrix", lightCamera.viewMatrix);
            shader->setMat4Uniform("lightProjectionMatrix", lightCamera.projMatrix);

            glm::mat4 projScaleBiasMatrix = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(0.5, 0.5, 0.5));
            shader->setMat4Uniform("lightScaleBiasMatrix", projScaleBiasMatrix);
        }

        if (_nocomparison) {
            shader->setIntUniform("useSmoothStep", _usesmoothstep);
        }

        shader->setFloatUniform("bias", _useBias ? _bias : 0.0f);

        {
            glActiveTexture(GL_TEXTURE0);  //текстурный юнит 0
            glBindSampler(0, _sampler);
            _labTex->bind();
            shader->setIntUniform("diffuseTex", 0);

            glActiveTexture(GL_TEXTURE1);  //текстурный юнит 1
            glBindTexture(GL_TEXTURE_2D, _depthTexId);
            glBindSampler(1, _isLinearSampler ?
                             (_nocomparison ? _depthSamplerLinear_nocomparison : _depthSamplerLinear) :
                             (_nocomparison ? _depthSampler_nocomparison : _depthSampler));
            shader->setIntUniform("shadowTex", 1);

            drawLab(shader, camera);
        }

        {
            glActiveTexture(GL_TEXTURE0);  //текстурный юнит 0
            glBindSampler(0, _sampler);
            _objTex->bind();
            shader->setIntUniform("diffuseTex", 0);

            glActiveTexture(GL_TEXTURE1);  //текстурный юнит 1
            glBindTexture(GL_TEXTURE_2D, _depthTexId);
            glBindSampler(1, _isLinearSampler ?
                             (_nocomparison ? _depthSamplerLinear_nocomparison : _depthSamplerLinear) :
                             (_nocomparison ? _depthSampler_nocomparison : _depthSampler));
            shader->setIntUniform("shadowTex", 1);

            drawObj(shader, camera);
        }

        //Рисуем маркеры для всех источников света
        {
            _markerShader->use();

            _markerShader->setMat4Uniform("mvpMatrix", camera.projMatrix * camera.viewMatrix * glm::translate(glm::mat4(1.0f), _light.position));
            _markerShader->setVec4Uniform("color", glm::vec4(_light.diffuse, 1.0f));
            _marker->draw();
        }

        //Отсоединяем сэмплер и шейдерную программу
        glBindSampler(0, 0);
        glUseProgram(0);
    }

    void drawLab(const ShaderProgramPtr& shader, const CameraInfo& camera)
    {
        shader->setMat4Uniform("modelMatrix", _lab->modelMatrix());
        shader->setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * _cube->modelMatrix()))));

        _lab->draw();
    }

    void drawObj(const ShaderProgramPtr& shader, const CameraInfo& camera)
    {
        shader->setMat4Uniform("modelMatrix", _cube->modelMatrix());
        shader->setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * _cube->modelMatrix()))));

        _cube->draw();

        shader->setMat4Uniform("modelMatrix", _sphere->modelMatrix());
        shader->setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * _sphere->modelMatrix()))));

        _sphere->draw();

        shader->setMat4Uniform("modelMatrix", _bunny->modelMatrix());
        shader->setMat3Uniform("normalToCameraMatrix", glm::transpose(glm::inverse(glm::mat3(camera.viewMatrix * _bunny->modelMatrix()))));

        _bunny->draw();
    }

};

int main()
{
    SampleApplication app;
    app.start();

    return 0;
}