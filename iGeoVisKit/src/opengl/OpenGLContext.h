// UTF-8 without BOM
#ifndef OPENGL_CONTEXT_H
#define OPENGL_CONTEXT_H

#include <functional>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gui/GLView.h"
#include "GLText.h"

// OpenGLContext 负责集中执行底层的 DrawCall
class OpenGLContext {
public:
	OpenGLContext() : m_view(nullptr) {}
	~OpenGLContext() { m_view = nullptr; }

	inline void bind(GLView* view) { m_view = view; }
	inline GLView* boundView() const { return m_view; }

	// 渲染命令类型
	enum class CmdType {
		ClearColorDepth,
		UseProgram,
		SetUniformMat4,
		SetUniform1f,
		SetUniform2f,
		SetUniform3f,
		SetUniform1i,
		SetUniform4f,
		BindVAO,
		BindBufferArray,
		ActiveTexture0,
		BindTexture2D,
		EnableBlend,
		DisableBlend,
		BlendFunc,
		EnableDepthTest,
		DisableDepthTest,
		EnableVertexAttribArray,
		VertexAttribPointer,
		BufferData,
		DrawArrays,
		DisableProgram,
		DrawText,
		EnablePointSmooth,
		DisablePointSmooth
	};

	struct Command {
		CmdType type;
		// 公共字段，按需使用
		float clearColorR = 0, clearColorG = 0, clearColorB = 0, clearColorA = 0;
		unsigned int program = 0;
		int uniformLoc = -1;
		glm::mat4 mat4Value{};
		float f1 = 0.0f; float f2x = 0.0f; float f2y = 0.0f; float f4x = 0.0f; float f4y = 0.0f; float f4z = 0.0f; float f4w = 0.0f; int i1 = 0;
		unsigned int vao = 0;
		unsigned int buffer = 0; // VBO id
		int drawMode = 0; // GL_POINTS / GL_LINES / GL_TRIANGLES / GL_TRIANGLE_STRIP
		int drawCount = 0;
		// texture
		unsigned int tex = 0;
		// blend func
		int blendSrc = 0; int blendDst = 0;
		// attrib
		unsigned int attribIndex = 0; int attribSize = 0; int attribType = 0; bool attribNormalized = false; int attribStride = 0; size_t attribOffset = 0;
		// buffer data
		std::vector<unsigned char> bufBytes; int bufUsage = 0;
		GLText* text = nullptr;
		int textX = 0;
		int textY = 0;
		std::string textStr;
	};

	// 执行一组命令，确保所有 DrawCall 在此函数内部进行
	inline void draw(const std::vector<Command>& commands) {
		if (!m_view) return;
		m_view->make_current();
		for (const auto &cmd : commands) {
			switch (cmd.type) {
			case CmdType::ClearColorDepth:
				glClearColor(cmd.clearColorR, cmd.clearColorG, cmd.clearColorB, cmd.clearColorA);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				break;
			case CmdType::UseProgram:
				glUseProgram(cmd.program);
				break;
			case CmdType::SetUniformMat4:
				glUniformMatrix4fv(cmd.uniformLoc, 1, GL_FALSE, glm::value_ptr(cmd.mat4Value));
				break;
			case CmdType::SetUniform1f:
				glUniform1f(cmd.uniformLoc, cmd.f1);
				break;
			case CmdType::SetUniform2f:
				glUniform2f(cmd.uniformLoc, cmd.f2x, cmd.f2y);
				break;
			case CmdType::SetUniform3f:
				glUniform3f(cmd.uniformLoc, cmd.f4x, cmd.f4y, cmd.f4z);
				break;
			case CmdType::SetUniform1i:
				glUniform1i(cmd.uniformLoc, cmd.i1);
				break;
			case CmdType::SetUniform4f:
				glUniform4f(cmd.uniformLoc, cmd.f4x, cmd.f4y, cmd.f4z, cmd.f4w);
				break;
			case CmdType::BindVAO:
				glBindVertexArray(cmd.vao);
				break;
			case CmdType::BindBufferArray:
				glBindBuffer(GL_ARRAY_BUFFER, cmd.buffer);
				break;
			case CmdType::ActiveTexture0:
				glActiveTexture(GL_TEXTURE0);
				break;
			case CmdType::BindTexture2D:
				glBindTexture(GL_TEXTURE_2D, cmd.tex);
				break;
			case CmdType::EnableBlend:
				glEnable(GL_BLEND);
				break;
			case CmdType::DisableBlend:
				glDisable(GL_BLEND);
				break;
			case CmdType::BlendFunc:
				glBlendFunc(cmd.blendSrc, cmd.blendDst);
				break;
			case CmdType::EnableDepthTest:
				glEnable(GL_DEPTH_TEST);
				break;
			case CmdType::DisableDepthTest:
				glDisable(GL_DEPTH_TEST);
				break;
			case CmdType::EnableVertexAttribArray:
				glEnableVertexAttribArray(cmd.attribIndex);
				break;
			case CmdType::VertexAttribPointer:
				glVertexAttribPointer(cmd.attribIndex, cmd.attribSize, cmd.attribType, cmd.attribNormalized ? GL_TRUE : GL_FALSE, cmd.attribStride, (const void*)cmd.attribOffset);
				break;
			case CmdType::BufferData:
				glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd.bufBytes.size(), cmd.bufBytes.data(), cmd.bufUsage);
				break;
			case CmdType::DrawArrays:
				glDrawArrays(cmd.drawMode, 0, cmd.drawCount);
				break;
			case CmdType::DisableProgram:
				glUseProgram(0);
				break;
			case CmdType::DrawText:
				if (cmd.text) {
					cmd.text->draw_string(cmd.textX, cmd.textY, "%s", cmd.textStr.c_str());
				}
				break;
			case CmdType::EnablePointSmooth:
				glEnable(GL_POINT_SMOOTH);
				break;
			case CmdType::DisablePointSmooth:
				glDisable(GL_POINT_SMOOTH);
				break;
			}
		}
		glBindVertexArray(0);
		m_view->swap();
	}

private:
	GLView* m_view;
};

#endif // OPENGL_CONTEXT_H