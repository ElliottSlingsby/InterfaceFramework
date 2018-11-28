#include <iostream>

#include "Interface.hpp"

class System1 : public SystemInterface {
	std::string _name;

public:
	System1(const std::string& name = "") : _name(name) {
		Engine::subscribe<System1, INTERFACE_FUNC(Engine, SystemInterface::systemEvent1)>(1);
		Engine::subscribe<System1, INTERFACE_FUNC(Engine, SystemInterface::systemEvent2)>(0);
		Engine::subscribe<System1, INTERFACE_FUNC(Engine, SystemInterface::systemEvent3)>(0);
		Engine::subscribe<System1, INTERFACE_FUNC(Engine, SystemInterface::systemEvent4)>(0);
	}

	void systemEvent1() final {
		std::cout << "System1 event 1" << _name << std::endl;
	}

	void systemEvent2() final {
		std::cout << "System1 event 2" << _name << std::endl;
	}

	void systemEvent3() final {
		std::cout << "System1 event 3" << _name << std::endl;
	}

	void systemEvent4() final {
		std::cout << "System1 event 4" << _name << std::endl;
	}
};

class System2 : public SystemInterface {
	std::string _name;

public:
	System2(const std::string& name = "") : _name(name) {
		Engine::subscribe<System2, INTERFACE_FUNC(Engine, SystemInterface::systemEvent1)>(0);
		Engine::subscribe<System2, INTERFACE_FUNC(Engine, SystemInterface::systemEvent2)>(0);
		Engine::subscribe<System2, INTERFACE_FUNC(Engine, SystemInterface::systemEvent3)>(0);
		Engine::subscribe<System2, INTERFACE_FUNC(Engine, SystemInterface::systemEvent4)>(0);
	}

	void systemEvent1() final {
		std::cout << "System2 event 1" << _name << std::endl;
	}

	void systemEvent2() final {
		std::cout << "System2 event 2" << _name << std::endl;
	}

	void systemEvent3() final {
		std::cout << "System2 event 3" << _name << std::endl;
	}

	void systemEvent4() final {
		std::cout << "System2 event 4" << std::endl;
	}
};

class System3 : public SystemInterface {
	std::string _name;

public:
	System3(const std::string& name = "") : _name(name) {
		Engine::subscribe<System3, INTERFACE_FUNC(Engine, SystemInterface::systemEvent1)>(-1);
		Engine::subscribe<System3, INTERFACE_FUNC(Engine, SystemInterface::systemEvent2)>(0);
		Engine::subscribe<System3, INTERFACE_FUNC(Engine, SystemInterface::systemEvent3)>(0);
		Engine::subscribe<System3, INTERFACE_FUNC(Engine, SystemInterface::systemEvent4)>(0);
	}

	void systemEvent1() final {
		std::cout << "System3 event 1" << _name << std::endl;
	}

	void systemEvent2() final {
		std::cout << "System3 event 2" << _name << std::endl;
	}

	void systemEvent3() final {
		std::cout << "System3 event 3" << _name << std::endl;
	}

	void systemEvent4() final {
		std::cout << "System3 event 4" << _name << std::endl;
	}
};

class Component1 : public ComponentInterface {
	std::string _name;

public:
	Component1(const std::string& name = "") : _name(name) {
		Engine::subscribe<Component1, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent1)>(0);
		Engine::subscribe<Component1, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent2)>(0);
		Engine::subscribe<Component1, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent3)>(0);
		Engine::subscribe<Component1, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent4)>(0);
	}

	void componentEvent1() final {
		std::cout << "Component1 event 1 " << _name << std::endl;
	}

	void componentEvent2() final {
		std::cout << "Component1 event 2 " << _name << std::endl;
	}

	void componentEvent3() final {
		std::cout << "Component1 event 3 " << _name << std::endl;
	}

	void componentEvent4() final {
		std::cout << "Component1 event 4 " << _name << std::endl;
	}
};

class Component2 : public ComponentInterface {
	std::string _name;

public:
	Component2(const std::string& name = "") : _name(name) {
		Engine::subscribe<Component2, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent1)>(0);
		Engine::subscribe<Component2, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent2)>(0);
		Engine::subscribe<Component2, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent3)>(0);
		Engine::subscribe<Component2, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent4)>(0);
	}

	void componentEvent1() final {
		std::cout << "Component2 event 1 " << _name << std::endl;
	}

	void componentEvent2() final {
		std::cout << "Component2 event 2 " << _name << std::endl;
	}

	void componentEvent3() final {
		std::cout << "Component2 event 3 " << _name << std::endl;
	}

	void componentEvent4() final {
		std::cout << "Component2 event 4 " << _name << std::endl;
	}
};

class Component3 : public ComponentInterface {
	std::string _name;

public:
	Component3(const std::string& name = "") : _name(name) {
		Engine::subscribe<Component3, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent1)>(0);
		Engine::subscribe<Component3, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent2)>(0);
		Engine::subscribe<Component3, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent3)>(0);
		Engine::subscribe<Component3, INTERFACE_FUNC(Engine, ComponentInterface::componentEvent4)>(0);
	}

	void componentEvent1() final {
		std::cout << "Component3 event 1 " << _name << std::endl;
	}

	void componentEvent2() final {
		std::cout << "Component3 event 2 " << _name << std::endl;
	}

	void componentEvent3() final {
		std::cout << "Component3 event 3 " << _name << std::endl;
	}

	void componentEvent4() final {
		std::cout << "Component3 event 4 " << _name << std::endl;
	}
};

int main(int argc, char** argv){
	Engine engine;

	engine.addSystem<System1>();
	engine.addSystem<System2>();
	engine.addSystem<System3>();
	
	engine.callSystems<INTERFACE_FUNC(Engine, SystemInterface::systemEvent1)>();
	engine.callSystems<INTERFACE_FUNC(Engine, SystemInterface::systemEvent2)>();
	engine.callSystems<INTERFACE_FUNC(Engine, SystemInterface::systemEvent3)>();
	
	std::vector<uint64_t> ids;

	for (uint32_t i = 0; i < 4; i++) {
		uint64_t id = engine.createEntity();

		const std::string number = std::to_string(i);

		engine.addComponent<Component1>(id, "entity " + number);
		engine.addComponent<Component2>(id, "entity " + number);
		engine.addComponent<Component3>(id, "entity " + number);

		ids.push_back(id);
	}

	engine.iterateEntities([](Engine::Entity& entity) {
		entity.call<INTERFACE_FUNC(Engine, ComponentInterface::componentEvent1)>();
	});

	std::cin.get();

	return 0;
}