// UTF-8 without BOM
#ifndef RENDERER_H
#define RENDERER_H


#include "opengl/OpenGLContext.h"

class FeatureSpaceGL;
class ImageGL;
class OverviewGL;
class GLText;

// Renderer 统一拼装绘制命令，并调用 OpenGLContext::draw 执行
class Renderer {
public:
	inline Renderer() {}
	inline ~Renderer() {}

	void setDebugShowTileOutlines(bool enabled) { m_debugShowTileOutlines = enabled; }

	void renderFeatureSpace(FeatureSpaceGL* fs, OpenGLContext& ctx);

	void renderImageScene(ImageGL* igl, OpenGLContext& ctx) const;

	void renderOverview(OverviewGL* ov, OpenGLContext& ctx);

private:
	bool m_debugShowTileOutlines = false;
};

#endif // RENDERER_H