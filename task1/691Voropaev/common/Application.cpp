#include "Application.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

#include "Common.h"

//======================================

//Функция обратного вызова для обработки нажатий на клавиатуре
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    app->handleKey(key, scancode, action, mods);
}

void windowSizeChangedCallback(GLFWwindow* window, int width, int height)
{
}

void mouseButtonPressedCallback(GLFWwindow* window, int button, int action, int mods)
{
}

void mouseCursosPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    app->handleMouseMove(xpos, ypos);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    app->handleScroll(xoffset, yoffset);
}

//======================================

Application::Application() :
        _camera_orbit_Mover(std::make_shared<FreeCameraMover>()),
        _camera_pov_Mover(std::make_shared<FreeCameraMover>()),
        is_Camera_orbit(true)
{
}

Application::~Application()
{
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
}

void Application::start()
{
    initContext();

    initGL();

    initGUI();

    makeScene();

    run();
}

void Application::initContext()
{
    if (!glfwInit())
    {
        std::cerr << "ERROR: could not start GLFW3\n";
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    int monitorsCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorsCount);

	// The last monitor ever we choose.
	GLFWmonitor* demoMonitor = monitors[monitorsCount - 1];
	const GLFWvidmode* videoMode = glfwGetVideoMode(demoMonitor);
	// Create fullscreen window on the chosen monitor.
	_window = glfwCreateWindow(videoMode->width, videoMode->height, "MIPT OpenGL demos", nullptr, nullptr);
	// Create default window on default monitor.
//    _window = glfwCreateWindow(800, 600, "MIPT OpenGL demos", nullptr, nullptr);

    if (!_window)
    {
        std::cerr << "ERROR: could not open window with GLFW3\n";
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(_window);

    glfwSwapInterval(0); //Отключаем вертикальную синхронизацию

    glfwSetWindowUserPointer(_window, this); //Регистрируем указатель на данный объект, чтобы потом использовать его в функциях обратного вызова}

    glfwSetKeyCallback(_window, keyCallback); //Регистрирует функцию обратного вызова для обработки событий клавиатуры
    glfwSetWindowSizeCallback(_window, windowSizeChangedCallback);
    glfwSetMouseButtonCallback(_window, mouseButtonPressedCallback);
    glfwSetCursorPosCallback(_window, mouseCursosPosCallback);
    glfwSetScrollCallback(_window, scrollCallback);
}

void Application::initGL()
{
    glewExperimental = GL_TRUE;
    glewInit();

    const GLubyte* renderer = glGetString(GL_RENDERER); //Получаем имя рендерера
    const GLubyte* version = glGetString(GL_VERSION); //Получаем номер версии
    const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL context version: " << version << std::endl;
    std::cout << "GLSL version: " << glslversion << std::endl;

    // Сбросим флаг ошибки.
    glGetError();

    if (GLEW_VERSION_4_5) {
        std::cout << "OpenGL 4.5 supported." << (USE_DSA ? " DSA explicitly enabled" : " DSA explicitly disabled") << (USE_INTERFACE_QUERY ? " Interface query explicitly enabled" : " Interface query explicitly disabled") << std::endl;
    }

	if (DebugOutput::isSupported())
		_debutOutput.attach();
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void Application::makeScene()
{
    _camera_orbit.viewMatrix = glm::lookAt(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    _camera_orbit.projMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);

    _camera_pov.viewMatrix = glm::lookAt(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    _camera_pov.projMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);

    if(is_Camera_orbit){
        _camera = &_camera_orbit;
    }
    else{
        _camera = &_camera_pov;
    }

}

void Application::run()
{
    while (!glfwWindowShouldClose(_window)) //Пока окно не закрыто
    {
        glfwPollEvents(); //Проверяем события ввода

        update(); //Обновляем сцену и положение виртуальной камеры

        updateGUI();

        draw(); //Рисуем один кадр

        drawGUI();

        glfwSwapBuffers(_window); //Переключаем передний и задний буферы
    }
    onStop();
}

void Application::handleKey(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(_window, GL_TRUE);
        }

        if (key == GLFW_KEY_Q)
        {
            is_Camera_orbit = !is_Camera_orbit;
            if(is_Camera_orbit){
                _camera = &_camera_orbit;
            }
            else{
                _camera = &_camera_pov;
            }
        }
    }


    if(is_Camera_orbit) {
        _camera_orbit_Mover->handleKey(_window, key, scancode, action, mods);
    }
    else{
        _camera_pov_Mover->handleKey(_window, key, scancode, action, mods);
    }
}

void Application::handleMouseMove(double xpos, double ypos)
{
    if (ImGui::IsMouseHoveringAnyWindow())
    {
        return;
    }

    if(is_Camera_orbit) {
        _camera_orbit_Mover->handleMouseMove(_window, xpos, ypos);
    }
    else{
        _camera_pov_Mover->handleMouseMove(_window, xpos, ypos);
    }
}

void Application::handleScroll(double xoffset, double yoffset)
{
    if(is_Camera_orbit) {
        _camera_orbit_Mover->handleScroll(_window, xoffset, yoffset);
    }
    else{
        _camera_pov_Mover->handleScroll(_window, xoffset, yoffset);
    }
}

void Application::update()
{
    double dt = glfwGetTime() - _oldTime;
    _oldTime = glfwGetTime();

    //-----------------------------------------

    if(is_Camera_orbit) {
        _camera_orbit_Mover->update(_window, dt);
        _camera_orbit = _camera_orbit_Mover->cameraInfo();
    }
    else{
        _camera_pov_Mover->update(_window, dt);
        _camera_pov = _camera_orbit_Mover->cameraInfo();
    }
}

void Application::draw()
{
}

//====================================================

void Application::initGUI()
{
    ImGui_ImplGlfwGL3_Init(_window, false);
}

void Application::updateGUI()
{
    ImGui_ImplGlfwGL3_NewFrame();
}

void Application::drawGUI()
{
    ImGui::Render();
}

