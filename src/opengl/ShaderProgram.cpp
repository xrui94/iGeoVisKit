#include "ShaderProgram.h"

#include <iostream>
#include <sstream>

ShaderProgram::ShaderProgram(const std::string& vertexShaderSource,
    const std::string& fragmentShaderSource)
{
    // 编译顶点和片段着色器
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // 链接程序
    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);
    checkLinkErrors(m_program);

    // 链接后可删除着色器对象（它们已被编译进程序）
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

ShaderProgram::~ShaderProgram()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : m_program(other.m_program)
{
    other.m_program = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    if (this != &other)
    {
        if (m_program != 0)
            glDeleteProgram(m_program);

        m_program = other.m_program;
        other.m_program = 0;
    }
    return *this;
}

unsigned int ShaderProgram::compileShader(unsigned int type, const std::string& source)
{
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    checkCompileErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    return shader;
}

void ShaderProgram::checkCompileErrors(unsigned int shader, const std::string& type)
{
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        int maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::string errorLog;
        errorLog.resize(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog.data());

        glDeleteShader(shader); // 删除无效着色器

        std::ostringstream oss;
        oss << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
            << errorLog << "\n-- --------------------------------------------------- --";
        throw std::runtime_error(oss.str());
    }
}

void ShaderProgram::checkLinkErrors(unsigned int program)
{
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::string errorLog;
        errorLog.resize(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, errorLog.data());

        glDeleteProgram(program); // 删除无效程序

        std::ostringstream oss;
        oss << "ERROR::PROGRAM_LINKING_ERROR\n"
            << errorLog << "\n-- --------------------------------------------------- --";
        throw std::runtime_error(oss.str());
    }
}

// 缓存 uniform location（避免重复 glGetUniformLocation）
int ShaderProgram::getUniformLocation(const std::string& name) const
{
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end())
        return it->second;

    int location = glGetUniformLocation(m_program, name.c_str());
    m_uniformLocations[name] = location;

    // 可选：调试时警告不存在的 uniform
#ifdef _DEBUG
    if (location == -1)
    {
        std::cerr << "Warning: uniform '" << name << "' not found in shader program.\n";
    }
#endif

    return location;
}

void ShaderProgram::setInt(const std::string& name, int value) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform1i(loc, value);
}

void ShaderProgram::setFloat(const std::string& name, float value) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform1f(loc, value);
}

void ShaderProgram::setBool(const std::string& name, bool value) const
{
    // OpenGL 中 bool uniform 用 glUniform1i 设置
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform1i(loc, value ? 1 : 0);
}

void ShaderProgram::setVec2(const std::string& name, const glm::vec2& value) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform2fv(loc, 1, glm::value_ptr(value));
}

void ShaderProgram::setVec3(const std::string& name, const glm::vec3& value) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform3fv(loc, 1, glm::value_ptr(value));
}

void ShaderProgram::setVec4(const std::string& name, const glm::vec4& value) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform4fv(loc, 1, glm::value_ptr(value));
}

void ShaderProgram::setMat4(const std::string& name, const glm::mat4& mat) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}
