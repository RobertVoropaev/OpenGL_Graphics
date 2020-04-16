#pragma once

#include <GL/glew.h>
#include <functional>

class DebugOutput {
public:
	typedef std::function<bool (GLenum, GLenum, GLuint, GLenum)> Filter;

	Filter filter;
public:
	DebugOutput();

	static bool isSupported() { return GLEW_KHR_debug; }

	void attach();

private:
	static void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
};


