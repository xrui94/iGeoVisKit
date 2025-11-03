#include <glad/glad.h>

#include "OpenGLContext.h"
#include "gui/GLView.h"
#include "GLText.h"

#include <glm/gtc/type_ptr.hpp>

//#include <functional>


OpenGLContext::OpenGLContext()
	: m_view(nullptr)
{
}

OpenGLContext::~OpenGLContext()
{
	m_view = nullptr;
}

// 执行一组命令，确保所有 DrawCall 在此函数内部进行
void OpenGLContext::draw(const std::vector<Command>& commands)
{
	if (!m_view) return;
	m_view->make_current();
	for (const auto& cmd : commands) {
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