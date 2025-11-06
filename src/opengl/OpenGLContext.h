// UTF-8 without BOM
#ifndef OPENGL_CONTEXT_H
#define OPENGL_CONTEXT_H

#include <glm/glm.hpp>

#include <string>
#include <vector>

class GLText;
//class GLView;

// OpenGLContext 负责集中执行底层的 DrawCall
class OpenGLContext {
public:
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
		float clearColorR = 0;
		float clearColorG = 0;
		float clearColorB = 0;
		float clearColorA = 0;

		//
		unsigned int program = 0;

		//
		int uniformLoc = -1;
		glm::mat4 mat4Value{};
		float f1 = 0.0f;
		float f2x = 0.0f;
		float f2y = 0.0f;
		float f4x = 0.0f;
		float f4y = 0.0f;
		float f4z = 0.0f;
		float f4w = 0.0f;
		int i1 = 0;
		unsigned int vao = 0;
		unsigned int buffer = 0; // VBO id
		int drawMode = 0; // GL_POINTS / GL_LINES / GL_TRIANGLES / GL_TRIANGLE_STRIP
		int drawCount = 0;

		// texture
		unsigned int tex = 0;

		// blend func
		int blendSrc = 0; int blendDst = 0;

		// attrib
		unsigned int attribIndex = 0;
		int attribSize = 0;
		int attribType = 0;
		bool attribNormalized = false;
		int attribStride = 0;
		size_t attribOffset = 0;

		// buffer data
		std::vector<unsigned char> bufBytes; int bufUsage = 0;
		GLText* text = nullptr;
		int textX = 0;
		int textY = 0;
		std::string textStr;
	};

public:
	OpenGLContext();
	~OpenGLContext();

	//inline void bind(GLView* view) { m_view = view; }
	//inline GLView* boundView() const { return m_view; }

	// 执行一组命令，确保所有 DrawCall 在此函数内部进行
	void draw(const std::vector<Command>& commands);

private:
	//GLView* m_view;
};

#endif // OPENGL_CONTEXT_H