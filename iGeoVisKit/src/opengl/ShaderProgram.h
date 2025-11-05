#pragma once

#include "PchApp.h"  // 假设已包含 <string>, <glad/glad.h> 等

#include <string>
#include <stdexcept>

class ShaderProgram
{
public:
    ShaderProgram(const std::string& vertexShaderSource,
        const std::string& fragmentShaderSource);

    ~ShaderProgram();

    // 禁止拷贝（OpenGL 对象不可复制）
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    // 允许移动（C++11 起推荐）
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

public:
    unsigned int getProgram() const { return m_program; }
    unsigned int getID() const { return m_program; }

    int getUniformLocation(const std::string& name) const;

    // 通用 uniform 设置（类型安全）
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setBool(const std::string& name, bool value) const;

    // 可选：方便绑定
    void use() const { glUseProgram(m_program); }

private:
    static unsigned int compileShader(unsigned int type, const std::string& source);
    static void checkCompileErrors(unsigned int shader, const std::string& type);
    static void checkLinkErrors(unsigned int program);

private:
    unsigned int m_program = 0;

    mutable std::unordered_map<std::string, int> m_uniformLocations;
};