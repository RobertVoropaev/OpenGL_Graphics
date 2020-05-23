#include "DebugOutput.h"

#include <unordered_map>
#include <iostream>
#include <cassert>
#include <string>

DebugOutput::DebugOutput() : filter([&] (GLenum source, GLenum type, GLuint id, GLenum severity) -> bool {
	// Accept all messages.
	// Ignore id 131218
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return false;

	return true;
}) { }

#define RECORD(enumPrefix, glenum) { GL_DEBUG_##enumPrefix##_##glenum, #glenum }
#define RECORD3(enumPrefix, glenum, val) { GL_DEBUG_##enumPrefix##_##glenum, val }

std::unordered_map<GLenum, std::string> debugSourceMapping = {
		RECORD(SOURCE, API),
		RECORD(SOURCE, WINDOW_SYSTEM),
		RECORD(SOURCE, SHADER_COMPILER),
		RECORD(SOURCE, THIRD_PARTY),
		RECORD(SOURCE, APPLICATION),
		RECORD(SOURCE, OTHER)
};

std::unordered_map<GLenum, std::string> debugTypeMapping = {
		RECORD(TYPE, ERROR),
		RECORD(TYPE, DEPRECATED_BEHAVIOR),
		RECORD(TYPE, UNDEFINED_BEHAVIOR),
		RECORD(TYPE, PORTABILITY),
		RECORD(TYPE, PERFORMANCE),
		RECORD(TYPE, MARKER),
		RECORD(TYPE, PUSH_GROUP),
		RECORD(TYPE, POP_GROUP),
		RECORD(TYPE, OTHER)
};

std::unordered_map<GLenum, std::string> debugSeverityMapping = {
		RECORD3(SEVERITY, HIGH, "high severity"),
		RECORD3(SEVERITY, MEDIUM, "medium severity"),
		RECORD3(SEVERITY, LOW, "low severity"),
		RECORD(SEVERITY, NOTIFICATION)
};

template <typename T>
const std::string getValueSafe(const std::unordered_map<T, std::string> &map, GLenum key) {
	auto iter = map.find(key);
	if (iter == map.cend())
		return std::to_string(static_cast<unsigned int>(key));
	return iter->second;
}

void GLAPIENTRY DebugOutput::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar *message, const void *userParam) {
	std::string sourceStr = getValueSafe(debugSourceMapping, source);
	std::string typeStr = getValueSafe(debugTypeMapping, type);
	std::string severityStr = getValueSafe(debugSeverityMapping, severity);

	// Filtering messages.
	const DebugOutput *debugOutput = (const DebugOutput*)(userParam);
	if (debugOutput && !debugOutput->filter(source, type, id, severity))
		return;

	std::cout << "[DEBUG] " << sourceStr << " " << typeStr << " " << severityStr << " #" << id << ": " << message << std::endl;

	// For debugging.
	if (type == GL_DEBUG_TYPE_ERROR)
		assert(false);
}

void DebugOutput::attach() {
	std::cout << "OpenGL debug output attached" << std::endl;
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Coherency.
	glDebugMessageCallback(DebugOutput::debugCallback, this);
}
