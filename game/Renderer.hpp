#pragma once

#include "SystemInterface.hpp"

#include "Window.hpp"

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <assimp\scene.h>

#include <vector>
#include <tuple>
#include <string>
#include <unordered_map>

inline void fromAssimp(const aiVector3D& from, glm::vec2* to) {
	to->x = from.x;
	to->y = from.y;
}

inline void fromAssimp(const aiVector3D& from, glm::vec3* to) {
	to->x = from.x;
	to->y = from.y;
	to->z = from.z;
}

inline void fromAssimp(const aiQuaternion& from, glm::quat* to) {
	to->w = from.w;
	to->x = from.x;
	to->y = from.y;
	to->z = from.z;
}


struct Model : public ComponentInterface {
	uint32_t programContextId = 0; // index + 1 into array in renderer
	uint32_t meshContextId = 0; // index + 1 into array in renderer

	uint32_t textureBufferId = 0; // opengl id

	uint32_t lightmapBufferId = 0;

	glm::uvec2 lightmapResolution = { 64, 64 }; // if 0 don't generate lightmaps

	Model(Engine& engine, uint64_t id) : ComponentInterface(engine, id) {
		INTERFACE_ENABLE(engine, ComponentInterface::serialize)(0);
	}

	void serialize(BaseReflector& reflector) final {
		reflector.buffer("model", "programContextId", &programContextId);
		reflector.buffer("model", "meshContextId", &meshContextId);
		reflector.buffer("model", "textureBufferId", &textureBufferId);
	}
};

class Renderer : public SystemInterface {
private:
	struct ProgramContext {
		GLuint program = 0;

		GLint modelUnifLoc = -1;
		GLint viewUnifLoc = -1;
		GLint projectionUnifLoc = -1;
		GLint modelViewUnifLoc = -1;
		GLint textureUnifLoc = -1;
		//GLint bonesUnifLoc = -1;

		std::string vertexFile = "";
		std::string fragmentFile = "";
	};

	struct MeshContext {
		GLuint arrayObject = 0;
		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;
		uint32_t indexCount = 0;

		std::string meshFile = "";
		uint32_t meshIndex = 0;
	};

public:
	struct ConstructorInfo {
		uint32_t positionAttrLoc = 0;
		uint32_t normalAttrLoc = 1;
		uint32_t texcoordAttrLoc = 2;
		//uint32_t tangentAttrLoc = 3;
		//uint32_t bitangentAttrLoc = 4;
		//uint32_t colourAttrLoc = 5;
		//uint32_t boneWeightsAttrLoc = 6; // un-used
		//uint32_t boneIndexesAttrLoc = 7; // un-used

		std::string modelUnifName = "model";
		std::string viewUnifName = "view";
		std::string projectionUnifName = "projection";
		std::string modelViewUnifName = "modelView";
		std::string textureUnifName = "texture";
		//std::string bonesUnifName = "bones"; // un-used
	};

	struct ShapeInfo {
		float verticalFov = 0.f;
		float zDepth = 0.f;
	};

private:
	Engine& _engine;

	const ConstructorInfo _constructionInfo;

	bool _rendering = false;

	ShapeInfo _shapeInfo;
	glm::vec2 _size;
	glm::mat4 _projectionMatrix;
	Engine::Entity _camera;

	std::vector<ProgramContext> _programContexts;
	std::vector<MeshContext> _meshContexts;

	std::unordered_map<std::string, GLuint> _textureFiles;
	//std::unordered_map<std::string, uint32_t> _meshFiles;
	std::unordered_map<std::string, GLuint> _shaderFiles;
	std::unordered_map<std::string, uint32_t> _programFiles;

	uint32_t _defaultProgram = 0;
	GLuint _defaultTexture = 0;
	
	Model* _addModel(uint64_t id, uint32_t mesh = 0, uint32_t texture = 0, GLuint program = 0);

	bool _compileShader(GLuint type, GLuint* shader, const std::string & file);

	void _bufferMesh(MeshContext* meshContext, const aiMesh& mesh);

	void _recusriveBufferMesh(const aiScene& scene, const aiNode& node, uint64_t parent, std::vector<uint32_t>* meshContextIds);

public:
	Renderer(Engine& engine, const ConstructorInfo& constructionInfo = ConstructorInfo());

	void initiate(const std::vector<std::string>& args) final;
	void update(double dt) final;
	void windowOpen(bool opened) final;
	void framebufferSize(glm::uvec2 size) final;

	void reshape(const ShapeInfo& config);
	void setCamera(uint64_t id);

	void buildLightmaps();

	void render(glm::uvec2 position = { 0, 0 }, glm::uvec2 size = { 0, 0 });

	uint32_t loadProgram(const std::string& vertexFile, const std::string& fragmentFile, uint64_t id = 0, bool reload = false);
	uint32_t loadTexture(const std::string& textureFile, uint64_t id = 0, bool reload = false);
	uint32_t loadMesh(const std::string& meshFile, uint64_t id = 0, bool reload = false);

	std::string vertexFile(uint32_t programId) const;
	std::string fragmentFile(uint32_t programId) const;
	std::string textureFile(uint32_t textureId) const;
	std::string meshFile(uint32_t meshId) const;

	void defaultProgram(const std::string& vertexFile, const std::string& fragmentFile);
	void defaultTexture(const std::string& textureFile);

	glm::mat4 viewMatrix() const;
};