#include "SystemInterface.hpp"

#include "Transform.hpp"
#include "Name.hpp"

#include "Window.hpp"
#include "Renderer.hpp"
#include "Controller.hpp"

#include "JsonReflector.hpp"

#include <iostream>
#include <filesystem>

#include "GlLoader.hpp"

void recursiveLoadTexture(Engine& engine, uint64_t id, const std::string& textureFile, bool reload = false) {
	Renderer& renderer = engine.system<Renderer>();

	Model* model = engine.getComponent<Model>(id);

	if (model)
		model->loadTexture(textureFile, reload);

	const Transform* transform = engine.getComponent<Transform>(id);

	if (!transform)
		return;

	std::vector<uint64_t> children;
	transform->getChildren(&children);

	for (uint64_t i : children)
		recursiveLoadTexture(engine, i, textureFile, reload);
}

uint64_t recursiveFindName(Engine& engine, uint64_t id, const std::string& str) {
	Name* name = engine.getComponent<Name>(id);

	if (name && *name == str)
		return id;

	Transform* transform = engine.getComponent<Transform>(id);

	if (!transform || !transform->hasChildren())
		return 0;

	std::vector<uint64_t> children;
	transform->getChildren(&children);

	for (uint64_t i : children) {
		uint64_t found = recursiveFindName(engine, i, str);

		if (found)
			return found;
	}

	return 0;
}

int main(int argc, char** argv) {
	Engine engine;

	engine.registerComponents<Transform, Name, Model>();

	Window::ConstructorInfo windowInfo;
	Renderer::ShaderVariableInfo newRendererInfo;

	engine.registerSystem<Window>(engine, windowInfo);
	engine.registerSystem<Controller>(engine);
	engine.registerSystem<Renderer>(engine, newRendererInfo);

	CALL_SYSTEMS(engine, SystemInterface::initiate)(std::vector<std::string>(argv, argv + argc));

	// ------------------- TESTING STATE BEGIN -------------------

	std::experimental::filesystem::path newPath = argv[0];
	const std::string path = newPath.replace_filename("data\\").string();

	Window& window = engine.system<Window>();
	Controller& controller = engine.system<Controller>();
	Renderer& renderer = engine.system<Renderer>();

	{
		// Window setup
		Window::WindowInfo windowConfig;

		window.openWindow(windowConfig);

		// Renderer setup
		Renderer::ShapeInfo shape;
		shape.verticalFov = 100;
		shape.zDepth = 100000;

		renderer.setShape(shape);
		renderer.setMainProgram("vertexShader.glsl", "fragmentShader.glsl");
	}

	// Camera
	{
		uint64_t id = engine.createEntity();

		engine.addComponent<Name>(id, "camera");

		Transform& transform = *engine.addComponent<Transform>(id);
		transform.position = { 0.f, -100.f, 100.f };
		transform.rotation = glm::quat({ glm::radians(90.f), 0.f, 0.f });

		renderer.setCamera(id);
		controller.setPossessed(id);
	}

	// Scene
	{
		uint64_t id = engine.createEntity();
	
		engine.addComponent<Name>(id, "scene");
	
		Transform& transform = *engine.addComponent<Transform>(id);
		transform.rotation = glm::quat({ glm::radians(90.f) , 0.f, 0.f });
		//transform.scale = { 10.f, 10.f, 10.f };
	
		renderer.loadScene("triangle_room.fbx", id);
		recursiveLoadTexture(engine, id, "checker.png");
	}

	// Skybox
	{
		uint64_t id = engine.createEntity();

		Name& name = *engine.addComponent<Name>(id, "skybox");

		Transform& transform = *engine.addComponent<Transform>(id);
		transform.scale = { 1000.f, 1000.f, 1000.f };

		Model& model = *engine.addComponent<Model>(id, "skybox.obj", "skybox.png");
	}
	
	//renderer.buildLightmaps();
	
	{
		JsonReflectorOut reflector(path + "test.json");

		engine.iterateEntities([&](Engine::Entity entity) {
			reflector.processEntity(engine, entity);
		});
	}
	
	std::cout << engine.entityCount() << '\n';
	
	// ------------------- TESTING STATE END -------------------

	TimePoint timer;
	double dt = 0.0;

	while (engine.running()) {
		startTime(&timer);

		CALL_SYSTEMS(engine, SystemInterface::update)(dt);
		CALL_SYSTEMS(engine, SystemInterface::lateUpdate)(dt);

		dt = deltaTime(timer);
	}

	return 0;
}