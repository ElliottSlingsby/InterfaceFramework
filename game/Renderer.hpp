#pragma once

#include "SystemInterface.hpp"

#include "GlLoader.hpp"

class Model : public ComponentInterface {
	GlLoader::MeshContext _meshContext;
	GlLoader::TextureContext _textureContext;

	std::string _meshFile;
	uint32_t _meshIndex;
	
	std::string _textureFile;

public:
	Model(Engine& engine, uint64_t id, const std::string& meshFile = "", const std::string& textureFile = "");

	void serialize(BaseReflector& reflector) final;

	void loadMesh(const std::string& meshFile, uint32_t meshIndex = 0, bool reload = false);
	void loadTexture(const std::string& textureFile, bool reload = false);

	const GlLoader::MeshContext& meshContext() const;
	const GlLoader::TextureContext& textureContext() const;

	friend class Renderer;
};

class Renderer : public SystemInterface {
public:
	struct ShaderVariableInfo {
		GlLoader::AttributeInfo attributes;

		struct UnfiromInfo {
			std::string modelUnifName = "model";
			std::string viewUnifName = "view";
			std::string projectionUnifName = "projection";
			std::string modelViewUnifName = "modelView";
			std::string textureUnifName = "texture";
			//std::string bonesUnifName = "bones"; // un-used
		} uniforms;
	};

	struct ProgramContextUniforms {
		GlLoader::ProgramContext programContext;

		GLint modelUnifLoc = -1;
		GLint viewUnifLoc = -1;
		GLint projectionUnifLoc = -1;
		GLint modelViewUnifLoc = -1;
		GLint textureUnifLoc = -1;
	};

	struct ShapeInfo {
		float verticalFov = 0.f;
		float zDepth = 0.f;
	};

private:
	Engine& _engine;

	ShaderVariableInfo::UnfiromInfo _uniforms;

	GlLoader _glLoader;

	std::string _path;

	ShapeInfo _shape;
	glm::vec2 _size;
	glm::mat4 _projectionMatrix;
	Engine::Entity _camera;

	ProgramContextUniforms _mainProgram;

	void _render(glm::uvec2 position = { 0, 0 }, glm::uvec2 size = { 0, 0 });

public:
	Renderer(Engine& engine, const ShaderVariableInfo& shaderVariables);

	void initiate(const std::vector<std::string>& args) final;
	void update(double dt) final;
	void framebufferSize(glm::uvec2 size) final;

	void setMainProgram(const std::string& vertexFile, const std::string& fragmentFile, bool reload = false);
	void setShape(const ShapeInfo& shape);
	void setCamera(uint64_t id);

	uint64_t loadScene(const std::string& meshFile, uint64_t id = 0, bool reload = false);

	glm::mat4 viewMatrix() const;

	GlLoader& loader();
	std::string path() const;
};