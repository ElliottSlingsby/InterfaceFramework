#include "TestFunctions.hpp"

#include "Transform.hpp"
#include "Renderer.hpp"
#include "Name.hpp"

uint64_t Test::createAxis(Engine& engine, const std::string& path, const glm::vec3& position, const glm::quat& rotation) {
	uint64_t id = engine.createEntity();

	Transform& transform = *engine.addComponent<Transform>(id);

	transform.position = position;
	transform.rotation = rotation;
	transform.scale = { 0.1f, 0.1f, 0.5f };

	transform.localTranslate(Transform::localForward * 7.5f); // poke the arrow through the mesh

	Renderer& renderer = engine.system<Renderer>();

	// CHANGE THIS! Assets should be instanced by filename!
	static uint32_t program = renderer.loadProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl");
	static uint32_t mesh = renderer.loadMesh(path + "arrow.obj");
	static GLuint texture = renderer.loadTexture(path + "arrow.png");

	Model& model = *engine.addComponent<Model>(id);

	model.meshContextId = mesh;
	model.programContextId = program;
	model.textureBufferId = texture;

	return id;
}

void Test::recursivelySetTexture(Engine& engine, uint64_t id, uint32_t textureBufferId) {
	Transform* transform = engine.getComponent<Transform>(id);
	Model* model = engine.getComponent<Model>(id);

	if (model)
		model->textureBufferId = textureBufferId;

	if (!transform || !transform->hasChildren())
		return;

	std::vector<uint64_t> children;
	transform->getChildren(&children);

	for (uint64_t i : children)
		recursivelySetTexture(engine, i, textureBufferId);
}

uint64_t Test::recursivelyFindName(Engine& engine, uint64_t id, const std::string& str) {
	Name* name = engine.getComponent<Name>(id);

	if (name && *name == str)
		return id;

	Transform* transform = engine.getComponent<Transform>(id);

	if (!transform || !transform->hasChildren())
		return 0;

	std::vector<uint64_t> children;
	transform->getChildren(&children);

	for (uint64_t i : children) {
		uint64_t found = recursivelyFindName(engine, i, str);

		if (found)
			return found;
	}

	return 0;
}