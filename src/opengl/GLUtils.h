#ifndef GL_UTILS_H
#define GL_UTILS_H

#include "PchApp.h"
#include "utils/Console.h"

// 命名空间采用小驼峰
namespace glUtils
{
	class GLUtils
	{
	public:
		// 返回GL错误码的字符串描述
		static const char* errorToString(GLenum err);

		// 清空当前GL错误队列
		static void clearErrors();

		// 检查并打印所有GL错误，返回是否无错误
		static bool checkErrors(const char* contextMsg = nullptr);

		// 带文件/行号的检查接口，便于定位
		static bool checkErrorsF(const char* contextMsg, const char* file, int line);
	};
}

// 宏命名遵循用户规范：全大写，以下划线开头和结尾
#define _GL_CHECK_(CTX) glUtils::GLUtils::checkErrorsF((CTX), __FILE__, __LINE__)

#endif // GL_UTILS_H