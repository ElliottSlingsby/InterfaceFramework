#include "Renderer.hpp"

#include <glad\glad.h>

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\matrix_decompose.hpp>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <stb_image.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#include "Transform.hpp"
#include "Name.hpp"

#include "TestFunctions.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

struct Vertex {
	glm::vec3 position;
	glm::vec2 texcoord;
};

struct Texel {
	glm::vec3 position;
	glm::vec3 tangent;
	glm::uvec2 texcoord;
};

inline void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string errorMessage(message, message + length);
	std::cerr << source << ',' << type << ',' << id << ',' << severity << std::endl << errorMessage << std::endl << std::endl;
}

inline float crossProduct(const glm::vec2& a, const glm::vec2& b) {
	return glm::cross(glm::vec3(a, 0), glm::vec3(b, 0)).z;
}

inline glm::vec3 baryInterpolate(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, float s, float t) {
	return v1 + s * (v2 - v1) + t * (v3 - v1);
}

template <typename T>
inline void interpolateTexels(Vertex triangle[3], const glm::vec2& resolution, const T& lambda) {
	// order triangle verts by y
	//std::sort(triangle, triangle + 3, [](const Vertex& a, const Vertex& b) -> bool {
	//	return a.texcoord.y < b.texcoord.y;
	//});

	// scale texcoords to resolution
	glm::vec2 vt1 = glm::clamp(triangle[0].texcoord, { 0.f, 0.f }, { 1.f, 1.f }) * resolution;
	glm::vec2 vt2 = glm::clamp(triangle[1].texcoord, { 0.f, 0.f }, { 1.f, 1.f }) * resolution;
	glm::vec2 vt3 = glm::clamp(triangle[2].texcoord, { 0.f, 0.f }, { 1.f, 1.f }) * resolution;

	// find triangle bounding box
	int minX = (int)glm::floor(glm::min(vt1.x, glm::min(vt2.x, vt3.x)));
	int maxX = (int)glm::ceil(glm::max(vt1.x, glm::max(vt2.x, vt3.x)));
	int minY = (int)glm::floor(glm::min(vt1.y, glm::min(vt2.y, vt3.y)));
	int maxY = (int)glm::ceil(glm::max(vt1.y, glm::max(vt2.y, vt3.y)));

	glm::vec2 vs1 = vt2 - vt1; // vt1 vertex to vt2
	glm::vec2 vs2 = vt3 - vt1; // vt1 vertex to vt3

	for (uint32_t x = minX; x < maxX; x++) {
		for (uint32_t y = minY; y < maxY; y++) {
			glm::vec2 q{ x - vt1.x, y - vt1.y };

			q += glm::vec2{ 0.5f, 0.5f };

			float area2 = crossProduct(vs1, vs2); // area * 2 of triangle

			// find barycentric coordinates
			float s = crossProduct(q, vs2) / area2;
			float t = crossProduct(vs1, q) / area2;

			glm::vec3 position = baryInterpolate(triangle[0].position, triangle[1].position, triangle[2].position, s, t);
			//glm::vec3 normal = baryInterpolate(triangle[0].normal, triangle[1].normal, triangle[2].normal, s, t);

			// if in triangle
			if ((s >= 0) && (t >= 0) && (s + t <= 1))
				lambda({ position, { x, y } });
		}
	}
}

Model* Renderer::_addModel(uint64_t id, uint32_t mesh, uint32_t texture, GLuint program) {
	Model& model = *_engine.addComponent<Model>(id);

	if (mesh)
		model.meshContextId = mesh;

	if (program)
		model.programContextId = program;
	else if (!model.programContextId && _defaultProgram)
		model.programContextId = _defaultProgram;

	if (texture)
		model.textureBufferId = texture;
	else if (!model.textureBufferId && _defaultTexture)
		model.textureBufferId = _defaultTexture;

	return &model;
}

bool Renderer::_compileShader(GLuint type, GLuint* shader, const std::string & file){
	if (*shader == 0)
		*shader = glCreateShader(type);

	std::ifstream stream;

	stream.open(file, std::ios::in);

	if (!stream.is_open())
		return false;

	const std::string source = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

	stream.close();

	const GLchar* sourcePtr = (const GLchar*)(source.c_str());

	glShaderSource(*shader, 1, &sourcePtr, 0);
	glCompileShader(*shader);

	GLint compiled;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_TRUE)
		return true;

	GLint length = 0;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &length);

	std::vector<GLchar> message(length);
	glGetShaderInfoLog(*shader, length, &length, &message[0]);

	glDeleteShader(*shader);
	*shader = 0;

	std::cerr << (char*)(&message[0]) << std::endl << std::endl;
	return false;
}

void Renderer::_bufferMesh(MeshContext* meshContext, const aiMesh& mesh){
	assert(meshContext); // sanity

	// gen buffers if new meshContext
	if (!meshContext->indexCount) {
		assert(!meshContext->arrayObject && !meshContext->indexBuffer && !meshContext->vertexBuffer); // sanity

		glGenVertexArrays(1, &meshContext->arrayObject);
		glGenBuffers(1, &meshContext->vertexBuffer);
		glGenBuffers(1, &meshContext->indexBuffer);
	}

	// bind buffers
	glBindVertexArray(meshContext->arrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshContext->indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshContext->vertexBuffer);

	// buffer index data
	meshContext->indexCount = mesh.mNumFaces * 3;

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshContext->indexCount * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

	for (uint32_t i = 0; i < mesh.mNumFaces; i++)
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i * sizeof(uint32_t) * 3, sizeof(uint32_t) * 3, mesh.mFaces[i].mIndices);

	// buffer vertex data
	size_t positionsSize = 3 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasPositions();
	size_t normalSize = 3 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasNormals();
	size_t textureCoordsSize = 2 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasTextureCoords(0);

	glBufferData(GL_ARRAY_BUFFER, positionsSize + normalSize + textureCoordsSize, 0, GL_STATIC_DRAW);

	// positions
	if (positionsSize) {
		glEnableVertexAttribArray(_constructionInfo.positionAttrLoc);
		glVertexAttribPointer(_constructionInfo.positionAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, mesh.mVertices);
	}

	// normals
	if (normalSize) {
		glEnableVertexAttribArray(_constructionInfo.normalAttrLoc);
		glVertexAttribPointer(_constructionInfo.normalAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionsSize));
		glBufferSubData(GL_ARRAY_BUFFER, positionsSize, normalSize, mesh.mNormals);
	}

	// texcoords (individually buffering vec3s to vec2s)
	if (textureCoordsSize) {
		glEnableVertexAttribArray(_constructionInfo.texcoordAttrLoc);
		glVertexAttribPointer(_constructionInfo.texcoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(positionsSize + normalSize));

		for (uint32_t i = 0; i < mesh.mNumVertices; i++)
			glBufferSubData(GL_ARRAY_BUFFER, positionsSize + normalSize + (i * 2 * sizeof(float)), 2 * sizeof(float), &mesh.mTextureCoords[0][i]);
	}
}

void Renderer::_recusriveBufferMesh(const aiScene& scene, const aiNode& node, uint64_t parent, std::vector<uint32_t>* meshContextIds){
	assert(meshContextIds); // sanity

	// if root then set id to parent, else create new one
	uint64_t id;

	if (scene.mRootNode == &node) {
		meshContextIds->resize(scene.mNumMeshes, 0);

		id = parent;
	}
	else if (parent) {
		id = _engine.createEntity();

		Transform& transform = *_engine.addComponent<Transform>(id);

		_engine.addComponent<Transform>(parent)->addChild(id);

		aiVector3D position, scale;
		aiQuaternion rotation;

		node.mTransformation.Decompose(scale, rotation, position);

		fromAssimp(position, &transform.position);
		fromAssimp(scale, &transform.scale);
		fromAssimp(rotation, &transform.rotation);
	}

	// buffer mesh if existing
	if (node.mNumMeshes) {
		uint32_t meshContextId = (*meshContextIds)[node.mMeshes[0]];

		if (!meshContextId) {
			meshContextId = _meshContexts.size() + 1;
			_meshContexts.resize(_meshContexts.size() + 1);

			(*meshContextIds)[node.mMeshes[0]] = meshContextId;

			_bufferMesh(&_meshContexts[meshContextId - 1], *scene.mMeshes[node.mMeshes[0]]);
		}

		if (parent) {
			_addModel(id, meshContextId);
			_engine.addComponent<Name>(id, node.mName.C_Str());
		}
	}

	// recurse
	for (uint32_t i = 0; i < node.mNumChildren; i++)
		_recusriveBufferMesh(scene, *node.mChildren[i], id, meshContextIds);
}

Renderer::Renderer(Engine& engine, const ConstructorInfo& constructionInfo) : _engine(engine), _constructionInfo(constructionInfo), _camera(engine){
	INTERFACE_ENABLE(engine, SystemInterface::initiate)(0);
	INTERFACE_ENABLE(engine, SystemInterface::update)(1);

	INTERFACE_ENABLE(engine, SystemInterface::windowOpen)(0);
	INTERFACE_ENABLE(engine, SystemInterface::framebufferSize)(0);
}

void Renderer::initiate(const std::vector<std::string>& args){
	glDebugMessageCallback(errorCallback, nullptr);

	stbi_set_flip_vertically_on_load(true);
}

void Renderer::windowOpen(bool opened){
	_rendering = opened;

	if (!opened)
		return;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DITHER);
}

void Renderer::framebufferSize(glm::uvec2 size){
	_size.x = size.x;
	_size.y = size.y;
}

void Renderer::update(double dt){
	if (!_rendering)
		return;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render({ 0, 0 }, _size);
}

void Renderer::reshape(const ShapeInfo& config){
	_shapeInfo.verticalFov = config.verticalFov;
	_shapeInfo.zDepth = config.zDepth;
}

void Renderer::setCamera(uint64_t id) {
	_camera.set(id);
}

void Renderer::buildLightmaps(){
	// testing container
	std::vector<std::pair<glm::vec3, glm::quat>> texels;

	for (uint32_t i = 0; i < _meshContexts.size(); i++) {
		// buffer vertex data from opengl
		const MeshContext& meshContext = _meshContexts[i];

		glBindVertexArray(meshContext.arrayObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshContext.indexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshContext.vertexBuffer);

		GLuint texcoordsEnabled = 0;
		glGetVertexAttribIuiv(_constructionInfo.texcoordAttrLoc, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &texcoordsEnabled);

		if (!texcoordsEnabled)
			continue;

		size_t texcoordsOffset = 0;
		glGetVertexAttribPointerv(_constructionInfo.texcoordAttrLoc, GL_VERTEX_ATTRIB_ARRAY_POINTER, (void**)&texcoordsOffset);

		uint32_t* indices = (uint32_t*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
		uint8_t* buffer = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

		glm::vec3* positions = (glm::vec3*)(buffer);
		glm::vec2* texcoords = (glm::vec2*)(buffer + texcoordsOffset);

		// find all entities with same mesh
		std::vector<uint64_t> entityIds;

		_engine.iterateEntities([&](Engine::Entity& entity) {
			if (!entity.has<Transform, Model>())
				return;

			const Model& model = *entity.get<Model>();

			if (model.meshContextId == i + 1)
				entityIds.push_back(entity.id());
		});
		
		// interpolate texel positions for each entity (due to different lightmap resolutions)
		for (uint64_t id : entityIds) {
			const Transform& transform = *_engine.getComponent<Transform>(id);
			const Model& model = *_engine.getComponent<Model>(id);

			glm::mat4 modelMatrix = transform.globalMatrix();

			// iterate over triangles in mesh
			for (uint32_t y = 0; y < meshContext.indexCount / 3; y++) {
				Vertex triangle[3];

				for (uint8_t x = 0; x < 3; x++) {
					uint32_t index = indices[y * 3 + x];
					triangle[x] = { positions[index], texcoords[index] };
				}

				glm::vec3 tangent = glm::cross(triangle[1].position - triangle[0].position, triangle[2].position - triangle[0].position);

				interpolateTexels(triangle, model.lightmapResolution, [&](const Vertex& vertex) {
					glm::vec3 worldPosition = modelMatrix * glm::vec4(vertex.position, 1);
					glm::quat worldRotation;// = glm::inverse(glm::lookAt(worldPosition, worldPosition + tangent, { 1.f, 1.f, 1.f }));
				
					texels.push_back({ worldPosition, worldRotation });
				});
			}
		}

		// free data from opengl
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}

	// testing, change this!
	for (auto i : texels) {
		Test::createAxis(_engine, "C:/Git Projects/InterfaceEngine/bin/data/", i.first, i.second);
	}
}

void Renderer::render(glm::uvec2 position, glm::uvec2 size){
	if (!size.x && !size.y)
		size = _size;

	if (_shapeInfo.verticalFov && _size.x && _size.y && _shapeInfo.zDepth)
		_projectionMatrix = glm::perspectiveFov(glm::radians(_shapeInfo.verticalFov), (float)size.x, (float)size.y, 1.f, _shapeInfo.zDepth);

	glViewport(position.x, position.y, size.x, size.y);

	_engine.iterateEntities([&](Engine::Entity& entity) {
		if (!entity.has<Transform, Model>())
			return;

		const Transform& transform = *entity.get<Transform>();
		Model& model = *entity.get<Model>();

		if (!model.meshContextId || !model.programContextId || !model.textureBufferId)
			return;

		const ProgramContext& program = _programContexts[model.programContextId - 1];

		glUseProgram(program.program);

		// projection matrix
		if (program.projectionUnifLoc != -1)
			glUniformMatrix4fv(program.projectionUnifLoc, 1, GL_FALSE, &_projectionMatrix[0][0]);

		// view matrix
		glm::dmat4 viewMatrix = Renderer::viewMatrix();

		if (program.viewUnifLoc != -1)
			glUniformMatrix4fv(program.viewUnifLoc, 1, GL_FALSE, &((glm::mat4)viewMatrix)[0][0]);

		// model matrix
		glm::dmat4 modelMatrix;

		if (program.modelViewUnifLoc != -1 || program.viewUnifLoc != -1) {
			modelMatrix = transform.globalMatrix();

			if (program.viewUnifLoc != -1)
				glUniformMatrix4fv(program.modelUnifLoc, 1, GL_FALSE, &((glm::mat4)modelMatrix)[0][0]);
		}

		// model view matrix
		if (program.modelViewUnifLoc != -1)
			glUniformMatrix4fv(program.modelViewUnifLoc, 1, GL_FALSE, &((glm::mat4)(viewMatrix * modelMatrix))[0][0]);

		// texture
		if (program.textureUnifLoc != -1) {
			glBindTexture(GL_TEXTURE_2D, model.textureBufferId);

			glUniform1i(program.textureUnifLoc, 0);
		}

		// mesh
		MeshContext& meshContext = _meshContexts[model.meshContextId - 1];

		glBindVertexArray(meshContext.arrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, meshContext.vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshContext.indexBuffer);

		glDrawElements(GL_TRIANGLES, meshContext.indexCount, GL_UNSIGNED_INT, 0);
	});
}

uint32_t Renderer::loadProgram(const std::string& vertexFile, const std::string& fragmentFile, uint64_t id, bool reload) {
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;

	if (_shaderFiles.find(vertexFile) != _shaderFiles.end())
		vertexShader = _shaderFiles[vertexFile];

	if (_shaderFiles.find(fragmentFile) != _shaderFiles.end())
		fragmentShader = _shaderFiles[fragmentFile];

	bool newProgram = !vertexShader || !fragmentShader;
	bool success = true;
	
	if (!vertexShader || reload) {
		success &= _compileShader(GL_VERTEX_SHADER, &vertexShader, vertexFile);
		_shaderFiles[vertexFile] = vertexShader;
	}
	
	if (!fragmentShader || reload) {
		success &= _compileShader(GL_FRAGMENT_SHADER, &fragmentShader, fragmentFile);
		_shaderFiles[fragmentFile] = fragmentShader;
	}
	
	if (!success)
		return 0;

	std::string programFiles = vertexFile + '/' + fragmentFile;
	uint32_t programIndex;
	
	if (_programFiles.find(programFiles) != _programFiles.end()) {
		programIndex = _programFiles[programFiles];
	}
	else {
		programIndex = _programContexts.size();
		_programContexts.resize(_programContexts.size() + 1);
	
		_programFiles[programFiles] = programIndex;
	}
	
	if (newProgram || reload) {
		ProgramContext& program = _programContexts[programIndex];
	
		if (!program.program) {
			program.program = glCreateProgram();
	
			glAttachShader(program.program, vertexShader);
			glAttachShader(program.program, fragmentShader);
	
			glLinkProgram(program.program);
		}
	
		GLint success;
		glGetProgramiv(program.program, GL_LINK_STATUS, &success);
	
		if (!success) {
			GLint length = 0;
			glGetProgramiv(program.program, GL_INFO_LOG_LENGTH, &length);
	
			std::vector<GLchar> message(length);
			glGetProgramInfoLog(program.program, length, &length, &message[0]);
	
			glDeleteProgram(program.program);
			program.program = 0;

			std::cerr << (char*)(&message[0]) << std::endl << std::endl;
			return 0;
		}
	
		program.modelUnifLoc = glGetUniformLocation(program.program, _constructionInfo.modelUnifName.c_str());
		program.viewUnifLoc = glGetUniformLocation(program.program, _constructionInfo.viewUnifName.c_str());
		program.projectionUnifLoc = glGetUniformLocation(program.program, _constructionInfo.projectionUnifName.c_str());
		program.modelViewUnifLoc = glGetUniformLocation(program.program, _constructionInfo.modelViewUnifName.c_str());
		program.textureUnifLoc = glGetUniformLocation(program.program, _constructionInfo.textureUnifName.c_str());
	}
	
	if (_engine.validEntity(id)) {
		Model& model = *_engine.addComponent<Model>(id);
		model.programContextId = programIndex + 1;
	}

	return programIndex + 1;
}

uint32_t Renderer::loadTexture(const std::string & textureFile, uint64_t id, bool reload){
	// check if texture already loaded
	auto iter = _textureFiles.find(textureFile);

	if (!reload && iter != _textureFiles.end()) {
		if (id)
			_addModel(id, 0, iter->second);

		return iter->second;
	}
	
	// if not, or reloading, load from file
	int width, height, channels;
	uint8_t* data = stbi_load(textureFile.c_str(), &width, &height, &channels, 4);

	if (!data)
		return 0;

	// buffer data to opengl
	GLuint textureBuffer;

	if (iter != _textureFiles.end())
		textureBuffer = iter->second;
	else
		glGenTextures(1, &textureBuffer);

	glBindTexture(GL_TEXTURE_2D, textureBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(data);

	if (id)
		_addModel(id, 0, textureBuffer);

	return textureBuffer;
}

uint32_t Renderer::loadMesh(const std::string& meshFile, uint64_t id, bool reload){
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(meshFile, aiProcessPreset_TargetRealtime_MaxQuality);

	if (!scene || !scene->mNumMeshes)
		return 0;

	std::vector<uint32_t> meshContextIds;

	_recusriveBufferMesh(*scene, *scene->mRootNode, id, &meshContextIds);

	return meshContextIds[0];
}

std::string Renderer::vertexFile(uint32_t programId) const {
	return "";
}

std::string Renderer::fragmentFile(uint32_t programId) const {
	return "";
}

std::string Renderer::textureFile(uint32_t textureId) const {
	return "";
}

std::string Renderer::meshFile(uint32_t meshId) const {
	return "";
}

void Renderer::defaultProgram(const std::string& vertexFile, const std::string& fragmentFile){
	_defaultProgram = loadProgram(vertexFile, fragmentFile);
}

void Renderer::defaultTexture(const std::string & textureFile){
	_defaultTexture = loadTexture(textureFile);
}

glm::mat4 Renderer::viewMatrix() const{
	glm::mat4 matrix;

	if (_camera.valid() && _camera.has<Transform>())
		matrix = glm::inverse(_camera.get<Transform>()->globalMatrix());

	return matrix;
}