#include "GLUtils.h"

using namespace glUtils;

const char* GLUtils::errorToString(GLenum err)
{
	switch (err) {
	case GL_NO_ERROR: return "GL_NO_ERROR";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	default: return "GL_UNKNOWN_ERROR";
	}
}

void GLUtils::clearErrors()
{
	while (glGetError() != GL_NO_ERROR) {}
}

bool GLUtils::checkErrors(const char* contextMsg)
{
	bool ok = true;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		ok = false;
		const char* msg = errorToString(err);
		if (contextMsg) {
			Console::write("(EE) GL error: %s (%d) | %s\n", msg, (int)err, contextMsg);
		} else {
			Console::write("(EE) GL error: %s (%d)\n", msg, (int)err);
		}
	}
	return ok;
}

bool GLUtils::checkErrorsF(const char* contextMsg, const char* file, int line)
{
	bool ok = true;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		ok = false;
		const char* msg = errorToString(err);
		if (contextMsg) {
			Console::write("(EE) GL error: %s (%d) | %s [%s:%d]\n", msg, (int)err, contextMsg, file, line);
		} else {
			Console::write("(EE) GL error: %s (%d) [%s:%d]\n", msg, (int)err, file, line);
		}
	}
	return ok;
}