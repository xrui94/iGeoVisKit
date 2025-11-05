#include "Renderer.h"

#include "opengl/OpenGLContext.h"
#include "opengl/FeatureSpaceGL.h"
#include "opengl/ImageGL.h"
#include "opengl/OverviewGL.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <cstring>

Renderer::Renderer()
{
	m_ctx = std::make_unique<OpenGLContext>();
}

Renderer::~Renderer()
{
}

void Renderer::renderFeatureSpace(FeatureSpaceGL* fs)
{
    if (!fs) return;
    // 确保在当前 GL 上下文中完成资源初始化
    fs->ensureGLResources();
    std::vector<OpenGLContext::Command> cmds;
    cmds.push_back({ OpenGLContext::CmdType::ClearColorDepth });
    cmds.back().clearColorR = 0.0f;
    cmds.back().clearColorG = 0.0f;
    cmds.back().clearColorB = 0.0f;
    cmds.back().clearColorA = 0.0f;

    // 基本状态
    cmds.push_back({ OpenGLContext::CmdType::EnableDepthTest });
    cmds.push_back({ OpenGLContext::CmdType::EnableBlend });
    OpenGLContext::Command bf{ OpenGLContext::CmdType::BlendFunc };
    bf.blendSrc = GL_SRC_ALPHA;
    bf.blendDst = GL_ONE_MINUS_SRC_ALPHA;
    cmds.push_back(bf);

    if (fs->gl_text) {
        OpenGLContext::Command t1{ OpenGLContext::CmdType::DrawText };
        t1.text = fs->gl_text;
        t1.textX = 5;
        t1.textY = 20;
        t1.textStr = std::string("Granularity: ") + std::to_string(fs->granularity) + std::string(":1");
        cmds.push_back(t1);

        OpenGLContext::Command t2{ OpenGLContext::CmdType::DrawText };
        t2.text = fs->gl_text;
        t2.textX = 5;
        t2.textY = 40;
        t2.textStr = std::string("Image Points: ") + std::to_string(fs->num_points);
        cmds.push_back(t2);

        OpenGLContext::Command t3{ OpenGLContext::CmdType::DrawText };
        t3.text = fs->gl_text;
        t3.textX = 5;
        t3.textY = 60;
        t3.textStr = std::string("Unique Points: ") + std::to_string(fs->vertices);
        cmds.push_back(t3);
    }

	//float aspect = view->aspect();
	float aspect = 1.f;
	glm::mat4 proj = glm::perspective(glm::radians(50.0f), aspect, 0.01f, 6.0f);
	glm::mat4 viewm = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(fs->cam_xpan, fs->cam_dolly + 0.5f, fs->cam_ypan));
	model = glm::rotate(model, glm::radians(fs->pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(fs->z_rot), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f, -0.5f, -0.5f));
	glm::mat4 mvp = proj * viewm * model;

	OpenGLContext::Command use{ OpenGLContext::CmdType::UseProgram };
	use.program = fs->program;
	cmds.push_back(use);

	OpenGLContext::Command setM{ OpenGLContext::CmdType::SetUniformMat4 };
	setM.uniformLoc = fs->loc_uMVP;
	setM.mat4Value = mvp;
	cmds.push_back(setM);

	OpenGLContext::Command bindBox{ OpenGLContext::CmdType::BindVAO };
	bindBox.vao = fs->vaoBox;
	cmds.push_back(bindBox);

	OpenGLContext::Command drawBox{ OpenGLContext::CmdType::DrawArrays };
	drawBox.drawMode = GL_LINES;
	drawBox.drawCount = fs->boxVertexCount;
	cmds.push_back(drawBox);
	cmds.push_back({
		fs->smooth ? OpenGLContext::CmdType::EnablePointSmooth : OpenGLContext::CmdType::DisablePointSmooth
	});

	for (size_t i = 0; i < fs->pointVaos.size(); ++i)
	{
		OpenGLContext::Command bindPts{ OpenGLContext::CmdType::BindVAO };
		bindPts.vao = fs->pointVaos[i];
		cmds.push_back(bindPts);

		OpenGLContext::Command drawPts{ OpenGLContext::CmdType::DrawArrays };
		drawPts.drawMode = GL_POINTS;
		drawPts.drawCount = fs->pointCounts[i];
		cmds.push_back(drawPts);
	}
	cmds.push_back({ OpenGLContext::CmdType::DisableProgram });
    m_ctx->draw(cmds);
}

void Renderer::renderImageScene(ImageGL* igl) const
{	
	// 确保纹理更新发生在当前 GL 上下文（由 GLView::paintGL 保证）
	if (igl) {
		igl->ensureGLResources();
		// 同步视口参数（x/y/width/height），供 check_textures 计算瓦片范围
		//igl->render_scene();
		// 仅在视口有效时才触发纹理检查，避免 zoom_level==0 导致 LOD 计算异常
		if (igl->viewport_width > 0 && igl->viewport_height > 0) {
			igl->ensureTexturesForRender();
		}
	}
	std::vector<OpenGLContext::Command> cmds;
	cmds.push_back({ OpenGLContext::CmdType::ClearColorDepth });
	cmds.back().clearColorR = 0.0f;
	cmds.back().clearColorG = 0.0f;
	cmds.back().clearColorB = 0.0f;
	cmds.back().clearColorA = 0.0f;

	// 如果视口无效，则仅清空后返回，避免后续计算与绘制出错
	if (!igl || igl->viewport_width <= 0 || igl->viewport_height <= 0) {
		m_ctx->draw(cmds);
		return;
	}

	// 调试输出：检查渲染参数
	printf("DEBUG: renderImageScene - viewport: x=%d, y=%d, w=%d, h=%d\n",
		igl->viewport_x, igl->viewport_y, igl->viewport_width, igl->viewport_height);
	printf("DEBUG: renderImageScene - tile range: row[%d-%d], col[%d-%d]\n",
		igl->viewport_start_row, igl->viewport_end_row, igl->viewport_start_col, igl->viewport_end_col);

	// 使用 GLM 的正交投影（像素空间到 NDC），与固定管线 glOrtho 等价
	//int vpX = igl->viewport_x;
	//int vpY = igl->viewport_y;
	//int vpW = igl->viewport_width;
	//int vpH = igl->viewport_height;
	int vpX = igl->viewport->get_image_x();
	int vpY = igl->viewport->get_image_y();
	int vpW = igl->viewport->get_viewport_width();
	int vpH = igl->viewport->get_viewport_height();
    glm::mat4 proj = glm::ortho((float)vpX, vpX + (float)vpW, vpY + (float)vpH, (float)vpY, 1.0f, -1.0f);

	bool drewAnyTile = false;
	printf("DEBUG: renderImageScene - Starting tile loop, tile_textures.size()=%zu\n", igl->tile_textures.size());
	// 图像瓦片绘制前，关闭深度测试，避免被深度状态影响
	cmds.push_back({ OpenGLContext::CmdType::DisableDepthTest });
	for (short y = (short)igl->viewport_start_row; y <= (short)igl->viewport_end_row; ++y)
	{
		for (short x = (short)igl->viewport_start_col; x <= (short)igl->viewport_end_col; ++x)
		{
			int tile_index = y * igl->tile_cols + x;
			if (tile_index < 0 || tile_index >= (int)igl->tile_textures.size()) {
				printf("DEBUG: renderImageScene - Skipping tile (%d,%d), index %d out of range\n", x, y, tile_index);
				continue;
			}
			GLuint tex_id = igl->tile_textures[tile_index];
			printf("DEBUG: renderImageScene - Tile (%d,%d) index=%d, tex_id=%u\n", x, y, tile_index, tex_id);
			if (tex_id == 0) continue;

			// 预计算本瓦片的缩放与偏移（图像像素），以及最终屏幕矩形，便于与 VS 一致性验证
			int tileW = (x == igl->tile_cols - 1) ? igl->last_column_width : igl->tile_image_size;
			int tileH = (y == igl->tile_rows - 1) ? igl->last_row_height : igl->tile_image_size;
			float offX = (float)(x * igl->tile_image_size);
			float offY = (float)(y * igl->tile_image_size);
			printf("DEBUG: tile(%d,%d) imgOff=(%.1f,%.1f) scale=(%d,%d) viewport=[%d,%d,%d,%d]\n",
				x, y, offX, offY, tileW, tileH, vpX, vpY, vpW, vpH);
			OpenGLContext::Command cUse{ OpenGLContext::CmdType::UseProgram };
			cUse.program = igl->glProgram;
			cmds.push_back(cUse);

			OpenGLContext::Command cAT{ OpenGLContext::CmdType::ActiveTexture0 };
			cmds.push_back(cAT);

			OpenGLContext::Command cBT{ OpenGLContext::CmdType::BindTexture2D };
			cBT.tex = tex_id;
			cmds.push_back(cBT);

			// 显式设置采样器到 0，消除驱动默认值差异
			if (igl->loc_uTex >= 0) {
				OpenGLContext::Command cUT{ OpenGLContext::CmdType::SetUniform1i };
				cUT.uniformLoc = igl->loc_uTex;
				cUT.i1 = 0;
				cmds.push_back(cUT);
			}

			// 设置正交投影矩阵（主图像程序）
			if (igl->loc_uProj >= 0) {
				OpenGLContext::Command cProj{ OpenGLContext::CmdType::SetUniformMat4 };
				cProj.uniformLoc = igl->loc_uProj;
				cProj.mat4Value = proj;
				cmds.push_back(cProj);
			}

			// 不再在 VS 中应用屏幕缩放与锚点，保持与固定管线一致
			// 将缩放作为 vec2 传入，支持非等比缩放
			if (igl->loc_uScale >= 0) {
				OpenGLContext::Command cScale{ OpenGLContext::CmdType::SetUniform2f };
				cScale.uniformLoc = igl->loc_uScale;
				//cScale.f2x = (float)tileW;
				//cScale.f2y = (float)tileH;
				cScale.f2x = (float)igl->tile_image_size;
				cScale.f2y = (float)igl->tile_image_size;
				cmds.push_back(cScale);
			}

			// 偏移为图像相对视口左上角的像素坐标（不乘缩放）
			if (igl->loc_uOffset >= 0) {
				OpenGLContext::Command cOff{ OpenGLContext::CmdType::SetUniform2f };
				cOff.uniformLoc = igl->loc_uOffset;
                //cOff.f2x = offX - (float)vpX;
                //cOff.f2y = offY - (float)vpY;
				cOff.f2x = x * (float)igl->tile_image_size;
				cOff.f2y = y * (float)igl->tile_image_size;
				cmds.push_back(cOff);
			}

			// 设置纹理坐标缩放：按数据在纹理中的有效比例，避免采样到零填充
			if (igl->loc_uTexScale >= 0) {
				float sX = (float)tileW / (float)igl->tile_image_size;
				float sY = (float)tileH / (float)igl->tile_image_size;
				OpenGLContext::Command cTS{ OpenGLContext::CmdType::SetUniform2f };
				cTS.uniformLoc = igl->loc_uTexScale;
				//cTS.f2x = sX;
				//cTS.f2y = sY;
				cTS.f2x = 1.f;
				cTS.f2y = 1.f;
				cmds.push_back(cTS);
			}

			// 移除地理仿射矩阵与原点设置，保持像素空间
			OpenGLContext::Command cBindVAO{ OpenGLContext::CmdType::BindVAO };
			cBindVAO.vao = igl->glVao;
			cmds.push_back(cBindVAO);

			OpenGLContext::Command cBindVBO{ OpenGLContext::CmdType::BindBufferArray };
			cBindVBO.buffer = igl->glVbo;
			cmds.push_back(cBindVBO);

			OpenGLContext::Command cDraw{ OpenGLContext::CmdType::DrawArrays };
			cDraw.drawMode = GL_TRIANGLES;
			cDraw.drawCount = 6;
			cmds.push_back(cDraw);

			// Debug: draw tile outline to验证几何位置（使用 ROI 程序）
			if (m_debugShowTileOutlines && igl->glRoiProgram != 0) {
                float x0 = (float)(x * igl->tile_image_size) - (float)vpX;
                float y0 = (float)(y * igl->tile_image_size) - (float)vpY;
                float x1 = x0 + (float)tileW;
                float y1 = y0 + (float)tileH;
				struct V2 {
					float x;
					float y;
				};
				V2 rect[4] = { {x0,y0}, {x1,y0}, {x1,y1}, {x0,y1} };
				OpenGLContext::Command rUse{ OpenGLContext::CmdType::UseProgram };
				rUse.program = igl->glRoiProgram;
				cmds.push_back(rUse);

				if (igl->locRoi_uProj >= 0) {
					OpenGLContext::Command rProj{ OpenGLContext::CmdType::SetUniformMat4 };
					rProj.uniformLoc = igl->locRoi_uProj;
					rProj.mat4Value = proj;
					cmds.push_back(rProj);
				}
				if (igl->locRoi_uColor >= 0) {
					OpenGLContext::Command rCol{ OpenGLContext::CmdType::SetUniform4f };
					rCol.uniformLoc = igl->locRoi_uColor;
					rCol.f4x = 0.0f;
					rCol.f4y = 1.0f;
					rCol.f4z = 0.0f;
					rCol.f4w = 0.25f;
					cmds.push_back(rCol);
				}
				OpenGLContext::Command rBindVBO{ OpenGLContext::CmdType::BindBufferArray };
				rBindVBO.buffer = igl->glRoiVbo;
				cmds.push_back(rBindVBO);

				OpenGLContext::Command rBD{ OpenGLContext::CmdType::BufferData };
				rBD.bufUsage = GL_DYNAMIC_DRAW;
				rBD.bufBytes.resize(sizeof(rect));
				memcpy(rBD.bufBytes.data(), rect, sizeof(rect));
				cmds.push_back(rBD);

				OpenGLContext::Command rBindVAO{ OpenGLContext::CmdType::BindVAO };
				rBindVAO.vao = igl->glRoiVao;
				cmds.push_back(rBindVAO);

				OpenGLContext::Command rEAttr{ OpenGLContext::CmdType::EnableVertexAttribArray };
				rEAttr.attribIndex = (GLuint)0;
				cmds.push_back(rEAttr);

				OpenGLContext::Command rVAP{ OpenGLContext::CmdType::VertexAttribPointer };
				rVAP.attribIndex = (GLuint)0;
				rVAP.attribSize = 2;
				rVAP.attribType = GL_FLOAT;
				rVAP.attribNormalized = false;
				rVAP.attribStride = sizeof(V2);
				rVAP.attribOffset = 0;
				cmds.push_back(rVAP);

				OpenGLContext::Command rDrawLoop{ OpenGLContext::CmdType::DrawArrays };
				rDrawLoop.drawMode = GL_LINE_LOOP;
				rDrawLoop.drawCount = 4;
				cmds.push_back(rDrawLoop);

				OpenGLContext::Command rDisable{ OpenGLContext::CmdType::DisableProgram };
				cmds.push_back(rDisable);
			}
			drewAnyTile = true;
		}
	}
	// 恢复深度测试（用于 ROI/其它对象）
	cmds.push_back({ OpenGLContext::CmdType::EnableDepthTest });
	OpenGLContext::Command cDisable{ OpenGLContext::CmdType::DisableProgram };
	cmds.push_back(cDisable);

	// 绘制 ROI（迁移到 Renderer + OpenGLContext 命令集）
	if (igl->roiset && igl->glRoiProgram != 0) {
		OpenGLContext::Command rUse{ OpenGLContext::CmdType::UseProgram };
		rUse.program = igl->glRoiProgram;
		cmds.push_back(rUse);
		if (igl->locRoi_uProj >= 0) {
			OpenGLContext::Command rProj{ OpenGLContext::CmdType::SetUniformMat4 };
			rProj.uniformLoc = igl->locRoi_uProj;
			rProj.mat4Value = proj;
			cmds.push_back(rProj);
		}
		// ROI 不再使用地理仿射与原点，保持像素空间
		if (igl->locRoi_uPointSize >= 0) {
			OpenGLContext::Command rPS{ OpenGLContext::CmdType::SetUniform1f };
			rPS.uniformLoc = igl->locRoi_uPointSize;
			rPS.f1 = 6.0f;
			cmds.push_back(rPS);
		}
		OpenGLContext::Command rBindVAO{ OpenGLContext::CmdType::BindVAO };
		rBindVAO.vao = igl->glRoiVao;
		cmds.push_back(rBindVAO);

		OpenGLContext::Command rBindVBO{ OpenGLContext::CmdType::BindBufferArray };
		rBindVBO.buffer = igl->glRoiVbo;
		cmds.push_back(rBindVBO);

		// 现有 ROI（按各自颜色绘制）
		std::vector<ROI*> rois = igl->roiset->get_regions();
		for (size_t i = 0; i < rois.size(); ++i)
		{
			ROI* roi = rois[i];
			if (!roi || !roi->get_active()) continue;
			int red = 128, green = 128, blue = 128;
			roi->get_colour(&red, &green, &blue);
			if (igl->locRoi_uColor >= 0) {
				OpenGLContext::Command rCol{ OpenGLContext::CmdType::SetUniform3f };
				rCol.uniformLoc = igl->locRoi_uColor;
				rCol.f4x = red / 255.0f;
				rCol.f4y = green / 255.0f;
				rCol.f4z = blue / 255.0f;
				cmds.push_back(rCol);
			}

			//
			std::vector<ROIEntity*> entities = roi->get_entities();
			for (size_t j = 0; j < entities.size(); ++j) {
				ROIEntity* e = entities[j];
				if (!e) continue;
				const char* type = e->get_type();
				std::vector<coords> pts = e->get_points();
				if (type == ROI_POLY) {
					if (pts.size() >= 3) {
						OpenGLContext::Command rBD{ OpenGLContext::CmdType::BufferData };
						rBD.bufUsage = GL_DYNAMIC_DRAW;
						rBD.bufBytes.resize(pts.size() * 2 * sizeof(float));
						float* out = reinterpret_cast<float*>(rBD.bufBytes.data());
                        for (size_t k = 0;
                            k < pts.size();
                            ++k) {
                            out[k * 2 + 0] = (float)pts[k].x - (float)vpX;
                            out[k * 2 + 1] = (float)pts[k].y - (float)vpY;
                        }
						cmds.push_back(rBD);

						OpenGLContext::Command rDraw{ OpenGLContext::CmdType::DrawArrays };
						rDraw.drawMode = GL_LINE_LOOP;
						rDraw.drawCount = (int)pts.size();
						cmds.push_back(rDraw);
					}
				}
				else if (type == ROI_POINT) {
					if (pts.size() >= 1) {
						OpenGLContext::Command rBD{ OpenGLContext::CmdType::BufferData };
						rBD.bufUsage = GL_DYNAMIC_DRAW;
						rBD.bufBytes.resize(2 * sizeof(float));
						float* out = reinterpret_cast<float*>(rBD.bufBytes.data());
                        out[0] = (float)pts[0].x - (float)vpX;
                        out[1] = (float)pts[0].y - (float)vpY;
						cmds.push_back(rBD);

						OpenGLContext::Command rDraw{ OpenGLContext::CmdType::DrawArrays };
						rDraw.drawMode = GL_POINTS;
						rDraw.drawCount = 1;
						cmds.push_back(rDraw);
					}
				}
				else if (type == ROI_RECT) {
					if (pts.size() >= 2) {
						coords tl = pts[0];
						coords br = pts[1];
                        float rect[8] = {
                            (float)tl.x - (float)vpX, (float)tl.y - (float)vpY, (float)tl.x - (float)vpX, (float)br.y - (float)vpY,
                            (float)br.x - (float)vpX, (float)br.y - (float)vpY, (float)br.x - (float)vpX, (float)tl.y - (float)vpY
                        };
						OpenGLContext::Command rBD{ OpenGLContext::CmdType::BufferData };
						rBD.bufUsage = GL_DYNAMIC_DRAW;
						rBD.bufBytes.resize(sizeof(rect));
						memcpy(rBD.bufBytes.data(), rect, sizeof(rect));
						cmds.push_back(rBD);

						OpenGLContext::Command rDraw{ OpenGLContext::CmdType::DrawArrays };
						rDraw.drawMode = GL_LINE_LOOP;
						rDraw.drawCount = 4;
						cmds.push_back(rDraw);
					}
				}
			}
		}

		// 正在编辑的 ROI（临时白色）
		if (igl->roiset->editing()) {
			ROIEntity* e = igl->roiset->get_current_entity();
			if (e) {
				if (igl->locRoi_uColor >= 0) {
					OpenGLContext::Command rCol{ OpenGLContext::CmdType::SetUniform3f };
					rCol.uniformLoc = igl->locRoi_uColor;
					rCol.f4x = 1.0f;
					rCol.f4y = 1.0f;
					rCol.f4z = 1.0f;
					cmds.push_back(rCol);
				}
				const char* type = e->get_type();
				std::vector<coords> pts = e->get_points();
				if (pts.empty()) { /* nothing */ }
				else if (type == ROI_POLY) {
					OpenGLContext::Command rBD{ OpenGLContext::CmdType::BufferData };
					rBD.bufUsage = GL_DYNAMIC_DRAW;
					rBD.bufBytes.resize((pts.size() + 1) * 2 * sizeof(float));
					float* out = reinterpret_cast<float*>(rBD.bufBytes.data());
                    for (size_t k = 0;
                        k < pts.size();
                        ++k) {
                        out[k * 2 + 0] = (float)pts[k].x - (float)vpX;
                        out[k * 2 + 1] = (float)pts[k].y - (float)vpY;
                    }
                    out[pts.size() * 2 + 0] = (float)igl->mouse_x - (float)vpX;
                    out[pts.size() * 2 + 1] = (float)igl->mouse_y - (float)vpY;
					cmds.push_back(rBD);

					OpenGLContext::Command rDraw{ OpenGLContext::CmdType::DrawArrays };
					rDraw.drawMode = GL_LINE_LOOP;
					rDraw.drawCount = (int)(pts.size() + 1);
					cmds.push_back(rDraw);
				}
				else if (type == ROI_RECT) {
                    float rect[8] = {
                        (float)pts[0].x - (float)vpX, (float)pts[0].y - (float)vpY, (float)pts[0].x - (float)vpX, (float)igl->mouse_y - (float)vpY,
                        (float)igl->mouse_x - (float)vpX, (float)igl->mouse_y - (float)vpY, (float)igl->mouse_x - (float)vpX, (float)pts[0].y - (float)vpY
                    };
					OpenGLContext::Command rBD{ OpenGLContext::CmdType::BufferData };
					rBD.bufUsage = GL_DYNAMIC_DRAW;
					rBD.bufBytes.resize(sizeof(rect));
					memcpy(rBD.bufBytes.data(), rect, sizeof(rect));
					cmds.push_back(rBD);

					OpenGLContext::Command rDraw{ OpenGLContext::CmdType::DrawArrays };
					rDraw.drawMode = GL_LINE_LOOP;
					rDraw.drawCount = 4;
					cmds.push_back(rDraw);
				}
				// POINT 类型在新建阶段只有 1 点，通常不显示拖拽
			}
		}

		cmds.push_back({ OpenGLContext::CmdType::DisableProgram });
	}

	//
	//if (!drewAnyTile && igl->gl_text) {
	//	OpenGLContext::Command t{ OpenGLContext::CmdType::DrawText };
	//	t.text = igl->gl_text;
	//	t.textX = 20;
	//	t.textY = 30;
	//	t.textStr = std::string("无图像");
	//	cmds.push_back(t);
	//}
	//if (igl->gl_text) {
	//	OpenGLContext::Command t{ OpenGLContext::CmdType::DrawText };
	//	t.text = igl->gl_text;
	//	t.textX = 10;
	//	t.textY = 20;
	//	t.textStr = std::string("Redrawing viewport at ") + std::to_string(igl->viewport_x) + std::string(",") + std::to_string(igl->viewport_y) + std::string(".");
	//	cmds.push_back(t);
	//}

	m_ctx->draw(cmds);
}

void Renderer::renderOverview(OverviewGL* ov)
{
	//if (!ov || !ctx.boundView()) return;
	//GLView* view = ctx.boundView();
	// 确保概览的 GL 资源与纹理在当前上下文中准备好
	if (ov) {
		ov->ensureGLResources();
		ov->make_texture();
	}
	std::vector<OpenGLContext::Command> cmds;
	cmds.push_back({ OpenGLContext::CmdType::ClearColorDepth });
	cmds.back().clearColorR = 0.0f;
	cmds.back().clearColorG = 0.0f;
	cmds.back().clearColorB = 0.0f;
	cmds.back().clearColorA = 0.0f;

	glm::mat4 proj = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f);
	OpenGLContext::Command cUseTex{ OpenGLContext::CmdType::UseProgram };
	cUseTex.program = ov->progTex;
	cmds.push_back(cUseTex);

	OpenGLContext::Command cProj{ OpenGLContext::CmdType::SetUniformMat4 };
	cProj.uniformLoc = ov->locTex_uProj;
	cProj.mat4Value = proj;
	cmds.push_back(cProj);

	OpenGLContext::Command cAT{ OpenGLContext::CmdType::ActiveTexture0 };
	cmds.push_back(cAT);

	OpenGLContext::Command cBT{ OpenGLContext::CmdType::BindTexture2D };
	cBT.tex = ov->tex_overview_id;
	cmds.push_back(cBT);

	OpenGLContext::Command cUT{ OpenGLContext::CmdType::SetUniform1i };
	cUT.uniformLoc = ov->locTex_uTex;
	cUT.i1 = 0;
	cmds.push_back(cUT);

	OpenGLContext::Command cBindTile{ OpenGLContext::CmdType::BindVAO };
	cBindTile.vao = ov->vaoTile;
	cmds.push_back(cBindTile);

	OpenGLContext::Command cDrawTile{ OpenGLContext::CmdType::DrawArrays };
	cDrawTile.drawMode = GL_TRIANGLES;
	cDrawTile.drawCount = 6;
	cmds.push_back(cDrawTile);

	OpenGLContext::Command cDisable{ OpenGLContext::CmdType::DisableProgram };
	cmds.push_back(cDisable);

	// 线段
	float maxDim = (float)std::max(ov->image_width, ov->image_height);
	float box_top = (float)ov->viewport->get_image_y();
	float box_bottom = (float)(ov->viewport->get_image_y() + ov->viewport->get_viewport_height());
	float box_left = (float)ov->viewport->get_image_x();
	float box_right = (float)ov->viewport->get_image_x() + (float)ov->viewport->get_viewport_width();
	if (box_top < 0.f) box_top = 0.f;
	if (box_bottom > (float)ov->image_height) box_bottom = (float)ov->image_height;
	if (box_left < 0.f) box_left = 0.f;
	if (box_right > (float)ov->image_width) box_right = (float)ov->image_width;
	struct V2 {
		float x, y;
	};
	std::vector<V2> lines;
	lines.push_back({ box_left / maxDim, box_top / maxDim });
	lines.push_back({ box_right / maxDim, box_top / maxDim });
	lines.push_back({ box_right / maxDim, box_bottom / maxDim });
	lines.push_back({ box_left / maxDim, box_bottom / maxDim });
	float halfway_point_x = (float)((box_left - box_right) / 2.0f);
	float halfway_point_y = (float)((box_top - box_bottom) / 2.0f);
	lines.push_back({ 0.0f, halfway_point_y / maxDim });
	lines.push_back({ box_left / maxDim, halfway_point_y / maxDim });
	lines.push_back({ (float)ov->image_width / maxDim, halfway_point_y / maxDim });
	lines.push_back({ box_right / maxDim, halfway_point_y / maxDim });
	lines.push_back({ halfway_point_x / maxDim, 0.0f });
	lines.push_back({ halfway_point_x / maxDim, box_top / maxDim });
	lines.push_back({ halfway_point_x / maxDim, (float)ov->image_height / maxDim });
	lines.push_back({ halfway_point_x / maxDim, box_bottom / maxDim });
	glm::mat4 projLines = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f);
	cmds.push_back({ OpenGLContext::CmdType::EnableBlend });

	OpenGLContext::Command cBF{ OpenGLContext::CmdType::BlendFunc };
	cBF.blendSrc = GL_SRC_ALPHA;
	cBF.blendDst = GL_ONE_MINUS_SRC_ALPHA;
	cmds.push_back(cBF);

	OpenGLContext::Command cUseColor{ OpenGLContext::CmdType::UseProgram };
	cUseColor.program = ov->progColor;
	cmds.push_back(cUseColor);

	OpenGLContext::Command cProjLines{ OpenGLContext::CmdType::SetUniformMat4 };
	cProjLines.uniformLoc = ov->locColor_uProj;
	cProjLines.mat4Value = projLines;
	cmds.push_back(cProjLines);

	OpenGLContext::Command cColor{ OpenGLContext::CmdType::SetUniform4f };
	cColor.uniformLoc = ov->locColor_uColor;
	cColor.f4x = 1.0f;
	cColor.f4y = 0.0f;
	cColor.f4z = 0.0f;
	cColor.f4w = 0.8f;
	cmds.push_back(cColor);

	OpenGLContext::Command cBindBL{ OpenGLContext::CmdType::BindBufferArray };
	cBindBL.buffer = ov->vboLines;
	cmds.push_back(cBindBL);

	OpenGLContext::Command cBD{ OpenGLContext::CmdType::BufferData };
	cBD.bufUsage = GL_DYNAMIC_DRAW;
	cBD.bufBytes.resize(lines.size() * sizeof(V2));
	memcpy(cBD.bufBytes.data(), lines.data(), cBD.bufBytes.size());
	cmds.push_back(cBD);

	OpenGLContext::Command cBindVL{ OpenGLContext::CmdType::BindVAO };
	cBindVL.vao = ov->vaoLines;
	cmds.push_back(cBindVL);

	OpenGLContext::Command cEAttr{ OpenGLContext::CmdType::EnableVertexAttribArray };
	cEAttr.attribIndex = (GLuint)ov->locColor_aPos;
	cmds.push_back(cEAttr);

	OpenGLContext::Command cVAP{ OpenGLContext::CmdType::VertexAttribPointer };
	cVAP.attribIndex = (GLuint)ov->locColor_aPos;
	cVAP.attribSize = 2;
	cVAP.attribType = GL_FLOAT;
	cVAP.attribNormalized = false;
	cVAP.attribStride = sizeof(V2);
	cVAP.attribOffset = 0;
	cmds.push_back(cVAP);

	OpenGLContext::Command cDrawLoop{ OpenGLContext::CmdType::DrawArrays };
	cDrawLoop.drawMode = GL_LINE_LOOP;
	cDrawLoop.drawCount = 4;
	cmds.push_back(cDrawLoop);

	OpenGLContext::Command cColor2{ OpenGLContext::CmdType::SetUniform4f };
	cColor2.uniformLoc = ov->locColor_uColor;
	cColor2.f4x = 1.0f;
	cColor2.f4y = 0.0f;
	cColor2.f4z = 0.0f;
	cColor2.f4w = 0.2f;
	cmds.push_back(cColor2);

	OpenGLContext::Command cDrawLines{ OpenGLContext::CmdType::DrawArrays };
	cDrawLines.drawMode = GL_LINES;
	cDrawLines.drawCount = (GLsizei)(lines.size() - 4);
	cmds.push_back(cDrawLines);

	cmds.push_back({ OpenGLContext::CmdType::DisableProgram });
	cmds.push_back({ OpenGLContext::CmdType::DisableBlend });
	m_ctx->draw(cmds);
}