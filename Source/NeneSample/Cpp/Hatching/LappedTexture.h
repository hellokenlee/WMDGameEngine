/*Copyright reserved by KenLee@2020 ken4000kl@gmail.com*/
#ifndef LAPPED_TEXTURE
#define LAPPED_TEXTURE

#include <set>
#include <deque>
#include <time.h>
#include <stdlib.h>

#include "NeneEngine/Debug.h"
#include "NeneEngine/Nene.h"

#include "LappedTexturePatch.h"


namespace lappedtexture
{
	struct CustomCBDS
	{
		NNVec3 tangent;
		NNFloat _padding0;
		NNVec4 color;
	};

	bool g_consecutive_add = true;
	bool g_consecutive_grow = true;
	bool g_shader_update = false;
	bool g_need_add_patch = false;
	bool g_need_grow_patch = false;
	bool g_need_optimaze_patch = false;
	int g_viewing_patch_index = 0;
	NNVec3 g_camera_pos;
	NNVec2 g_camera_rot;

	std::set<NNUInt> g_candidate_faces;

	std::shared_ptr<StaticMesh> g_bunny = nullptr;

	float g_tangent[3] = { 0.0f, 1.0f, 0.0f };

	std::vector<LappedTexturePatch> g_patches;
	std::vector<std::shared_ptr<Shader>> g_shaders;

	void KeyboardControl(std::shared_ptr<BaseEvent> eve)
	{
		std::shared_ptr<KeyboardEvent> k_event = std::dynamic_pointer_cast<KeyboardEvent>(eve);
		if (k_event->mKey == NNKeyMap(ESCAPE))
		{
			Utils::SetWindowShouldClose(true);
		}
		else if (k_event->mKey == NNKeyMap(F5))
		{
			g_shader_update = true;
		}
		else if(k_event->mKey == NNKeyMap(EQUAL))
		{
			g_need_add_patch = true;
		}
		else if (k_event->mKey == NNKeyMap(MINUS))
		{
			g_need_grow_patch = true;
		}
		else if (k_event->mKey == NNKeyMap(0))
		{
			g_need_optimaze_patch = true;
		}
	}

	void DrawGraphicUserInterfaces()
	{
		ImGui::Begin("Control", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		{
			//
			ImGui::SetWindowPos(ImVec2(10, 10));
			ImGui::SetWindowSize(ImVec2(320, 400));
			//
			ImGui::Text("Info:");
			ImGui::Text("%.2f ms/frame (%.0f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			//
			ImGui::Text("Camera: ");
			ImGui::Text("(%.1f, %.1f, %.1f) | (%.1f, %.1f) ", g_camera_pos.x, g_camera_pos.y, g_camera_pos.z, g_camera_rot.x, g_camera_rot.y);
			//
			ImGui::Text("Tangent: ");
			ImGui::SliderFloat3("  ", g_tangent, -1.0f, 1.0f);
			//
			ImGui::Text("Patch View: ");
			if (g_patches.size() == 0 || g_need_grow_patch)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			ImGui::SliderInt("    ", &g_viewing_patch_index, 0, int(g_patches.size()) - 1);
			if (g_patches.size() == 0 || g_need_grow_patch)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			//
			ImGui::Text("Patch Control: ");
			ImGui::Checkbox("Consecutive Grow", &g_consecutive_grow);
			ImGui::Checkbox("Add After Consecutive Grow", &g_consecutive_grow);
			if (ImGui::Button("Add Patch"))
			{
				g_need_add_patch = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Grow Patch"))
			{
				g_need_grow_patch = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Optimaze Patch"))
			{
				g_need_optimaze_patch = true;
			}
		}
		ImGui::End();
	}

	void InitPatch()
	{
		//
		g_candidate_faces.clear();
		//
		std::shared_ptr<Mesh> mesh = g_bunny->GetMeshes()[0];
		//
		std::vector<NNUInt>& indices = mesh->GetIndexData();
		std::vector<Vertex>& vertices = mesh->GetVertexData();
		//
		for (NNUInt f = 0; f < NNUInt(indices.size()) / 3; ++f)
		{
			g_candidate_faces.insert(f);
		}
	}
	
	void Main()
	{
		//
		srand((unsigned int)time(nullptr));
		//
		Utils::Init("LappedTexture", 1600, 900);
		Utils::ClearColor(0.1f, 0.1f, 0.1f);
		Keyboard::Instance().OnPress().AddCallbackFunc(KeyboardControl);
		//
		auto cc = CameraController::Create();
		auto ca = CoordinateAxes::Create(100.0f, 10.0f);
		auto controls = UserInterface::Create(DrawGraphicUserInterfaces);
		//
		cc->m_speed = 3.0f;
		cc->SetYaw(0.3f);
		cc->SetPitch(-0.3f);
		cc->SetPosition(NNVec3(-4.7f, 4.7f, -0.2f));
		//
		g_bunny = StaticMesh::Create("Resource/Mesh/bunny/bunny.obj", 30.0f);
		//
		auto shader_debug = Shader::Create("Resource/Shader/GLSL/Debug.vert", "Resource/Shader/GLSL/Debug.frag", NNVertexFormat::POSITION_NORMAL_TEXTURE, true);
		shader_debug->AddOptionalShader("Resource/Shader/GLSL/Debug.geom", NNShaderType::GEOMETRY_SHADER, true);
		g_shaders.push_back(shader_debug);
		auto shader_flat = Shader::Create("Resource/Shader/GLSL/Flat.vert", "Resource/Shader/GLSL/Flat.frag", NNVertexFormat::POSITION_NORMAL_TEXTURE);
		g_shaders.push_back(shader_flat);
		auto shader_patch = Shader::Create("Resource/Shader/GLSL/Patch.vert", "Resource/Shader/GLSL/Patch.frag", NNVertexFormat::POSITION_NORMAL_TEXTURE);
		g_shaders.push_back(shader_patch);
		auto shader_2d_color = Shader::Create("Resource/Shader/GLSL/2DColor.vert", "Resource/Shader/GLSL/2DColor.frag", NNVertexFormat::POSITION_NORMAL_TEXTURE);
		g_shaders.push_back(shader_2d_color);
		auto shader_2d_texture = Shader::Create("Resource/Shader/GLSL/2DTexture.vert", "Resource/Shader/GLSL/2DTexture.frag", NNVertexFormat::POSITION_NORMAL_TEXTURE);
		g_shaders.push_back(shader_2d_texture);
		auto shader_3d_color = Shader::Create("Resource/Shader/GLSL/3DColor.vert", "Resource/Shader/GLSL/3DColor.frag", NNVertexFormat::POSITION_NORMAL_TEXTURE);
		//
		auto texture_patch = Texture2D::Create("Resource/Texture/splotch_checkboard.png");
		//
		auto quad = Geometry::CreateQuad();
		//
		ConstantBuffer<LightCBDS> LightConstantBuffer;
		LightConstantBuffer.Data().ltype = 123.456f;
		LightConstantBuffer.Data().range = 1000.0f;
		LightConstantBuffer.Data().color = NNVec4(1.0f, 1.0f, 1.0f, 1.0f);
		LightConstantBuffer.Data().attenuation = 1000.0f;
		LightConstantBuffer.Data().position = NNVec4(3.0f, 4.0f, 4.0f, 0.0f);
		//
		ConstantBuffer<CustomCBDS> CustomConstantBuffer;
		//
		InitPatch();
		//
		while (!Utils::WindowShouldClose()) 
		{
			//
			Utils::Update();
			//
			cc->Update();
			cc->GetCamera()->Use();
			{
				NNVec3 tangent(g_tangent[0], g_tangent[1], g_tangent[2]);
				CustomConstantBuffer.Data().tangent = NNNormalize(tangent);
				CustomConstantBuffer.Update(NNConstantBufferSlot::CUSTOM_DATA_SLOT);
				LightConstantBuffer.Update(NNConstantBufferSlot::CUSTOM_LIGHT_SLOT);
				NeneCB::Instance().PerFrame().Update(NNConstantBufferSlot::PER_FRAME_SLOT);
			}
			
			//
			{
				//
				Utils::Clear();
				ca->Draw();
				
				g_bunny->Draw(shader_debug);
				// Draw the tex
				texture_patch->Use(0);
				quad->Draw(shader_2d_texture);
				// Draw the patches
				texture_patch->Use(0);
				shader_patch->Use();
				{
					for (auto& patch : g_patches)
					{
						patch.DrawMesh();
					}
				}

				/*
				{
					CustomConstantBuffer.Data().color = NNVec4(0.0, 1.0, 1.0, 1.0);
					CustomConstantBuffer.Update(NNConstantBufferSlot::CUSTOM_DATA_SLOT);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					g_bunny->Draw(shader_3d_color);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				*/
				
				// Draw viewing patch on texture for debug
				if (g_viewing_patch_index < g_patches.size())
				{
					texture_patch->Use(0);
					shader_2d_color->Use();
					{
						CustomConstantBuffer.Data().color = NNVec4(0.0, 1.0, 0.0, 1.0);
						CustomConstantBuffer.Update(NNConstantBufferSlot::CUSTOM_DATA_SLOT);
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						g_patches[g_viewing_patch_index].DrawMesh();
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}
				}

				if (g_viewing_patch_index < g_patches.size() && g_patches[g_viewing_patch_index].IsGrown())
				{
					g_patches[g_viewing_patch_index].CalcNotFullyCoveredFaces(g_candidate_faces);
				}

				//
				{
					g_camera_pos = cc->GetCamera()->GetPosition();
					g_camera_rot.x = cc->GetCamera()->GetYaw();
					g_camera_rot.y = cc->GetCamera()->GetPitch();
					controls->Draw();
				}
			}
			//
			Utils::SwapBuffers();
			
			//
			if (g_shader_update)
			{
				g_shader_update = false;
				printf("========== >>> Compiling Shaders >>> ===========\n");

				for (auto shader : g_shaders)
				{
					shader->Recompile();
				}

				printf("========== >>> Compiling Shaders <<< ===========\n");
			}
			if (g_need_add_patch && !g_need_grow_patch)
			{
				g_need_add_patch = false;
				//
				if (g_candidate_faces.size() > 0)
				{
					LappedTexturePatch patch;
					patch.Initialize(g_bunny->GetMeshes()[0], g_candidate_faces, texture_patch);
					g_patches.emplace_back(patch);
					g_viewing_patch_index = NNUInt(g_patches.size()) - 1;
				}
				if (g_consecutive_add)
				{
					g_need_grow_patch = true;
				}
			}
			if (g_need_grow_patch)
			{
				if (g_viewing_patch_index < g_patches.size())
				{
					LappedTexturePatch& patch = g_patches[g_viewing_patch_index];
					patch.Grow(g_candidate_faces);
					if (g_consecutive_grow)
					{
						if (patch.IsGrown())
						{
							g_need_grow_patch = false;
							if (g_consecutive_add)
							{
								g_need_add_patch = true;
							}
						}
					}
					else
					{
						g_need_grow_patch = false;
					}
				}
				else
				{
					g_need_grow_patch = false;
				}
			}
			if (g_need_optimaze_patch)
			{
				if (g_viewing_patch_index < g_patches.size())
				{
					LappedTexturePatch& patch = g_patches[g_viewing_patch_index];
					patch.Optimaze();
				}
				g_need_optimaze_patch = false;
			}

		}
		
		// 
		Utils::Terminate();
	}
}

#endif // LAPPED_TEXTURE