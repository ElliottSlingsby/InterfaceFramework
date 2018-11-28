#pragma once

#include "Engine.hpp"

class SystemInterface;
class ComponentInterface;

using Engine = SimpleEngine<SystemInterface, ComponentInterface>;

class SystemInterface : public Engine::BaseSystem {
public:
	virtual void systemEvent1() { }
	virtual void systemEvent2() { }
	virtual void systemEvent3() { }
	virtual void systemEvent4() { }
};

class ComponentInterface : public Engine::BaseComponent {
public:
	virtual void componentEvent1() { }
	virtual void componentEvent2() { }
	virtual void componentEvent3() { }
	virtual void componentEvent4() { }
};