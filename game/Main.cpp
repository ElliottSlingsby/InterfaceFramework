#include "SystemInterface.hpp"

#include "Transform.hpp"
#include "Name.hpp"

#include "Window.hpp"
#include "Renderer.hpp"
#include "Controller.hpp"

#include "TestFunctions.hpp"
#include "JsonReflector.hpp"

#include <iostream>
#include <filesystem>

int main(int argc, char** argv) {
	Engine engine;

	engine.registerComponents<Transform, Name, Model>();

	Window::ConstructorInfo windowInfo;
	Renderer::ConstructorInfo rendererInfo;

	engine.registerSystem<Window>(engine, windowInfo);
	engine.registerSystem<Controller>(engine);
	engine.registerSystem<Renderer>(engine, rendererInfo);

	CALL_SYSTEMS(engine, SystemInterface::initiate)(std::vector<std::string>(argv, argv + argc));

	// ------------------- TESTING STATE BEGIN -------------------

	std::experimental::filesystem::path newPath = argv[0];
	const std::string path = newPath.replace_filename("data/").string();

	Window& window = engine.system<Window>();
	Controller& controller = engine.system<Controller>();
	Renderer& renderer = engine.system<Renderer>();

	{
		// Window setup
		Window::WindowInfo windowConfig;

		window.openWindow(windowConfig);

		// Renderer setup
		Renderer::ShapeInfo shapeInfo;
		shapeInfo.verticalFov = 100;
		shapeInfo.zDepth = 100000;

		renderer.reshape(shapeInfo);
		renderer.defaultTexture(path + "checker.png");
		renderer.defaultProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl");
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

		renderer.loadMesh(path + "triangle_room.fbx", id);
	}

	// Skybox
	{
		uint64_t id = engine.createEntity();

		Name& name = *engine.addComponent<Name>(id, "123456789-123456789-123456789-123456789-");

		Transform& transform = *engine.addComponent<Transform>(id);
		transform.scale = { 1000.f, 1000.f, 1000.f };
	
		renderer.loadMesh(path + "skybox.obj", id);
		Test::recursivelySetTexture(engine, id, renderer.loadTexture(path + "skybox.png"));
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