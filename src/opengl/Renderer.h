// UTF-8 without BOM
#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

class OpenGLContext;
class FeatureSpaceGL;
class ImageGL;
class OverviewGL;
class GLText;

// Renderer 统一拼装绘制命令，并调用 OpenGLContext::draw 执行
class Renderer {
public:
	Renderer();
	~Renderer();

	void setDebugShowTileOutlines(bool enabled) { m_debugShowTileOutlines = enabled; }

	void renderFeatureSpace(FeatureSpaceGL* fs);

	void renderImageScene(ImageGL* igl) const;

	void renderOverview(OverviewGL* ov);

private:
	bool m_debugShowTileOutlines = false;

	std::unique_ptr<OpenGLContext> m_ctx;
};

#endif // RENDERER_H