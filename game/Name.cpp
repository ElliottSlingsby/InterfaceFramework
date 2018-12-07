#include "Name.hpp"

#include <cstring>

Name::Name(Engine& engine, uint64_t id, const std::string& str) : ComponentInterface(engine, id) {
	INTERFACE_ENABLE(engine, ComponentInterface::serialize)(0);

	set(str);
}

void Name::set(const std::string& str) {
	memset(_name, '\0', sizeof(_name));
	strcpy(_name, str.substr(0, NAME_LEN).c_str());
}

const char* Name::c_str() const {
	return _name;
}

bool Name::operator==(const std::string& str) const{
	return !strcmp(str.c_str(), _name);
}

void Name::serialize(BaseReflector & reflector) {
	reflector.buffer("name", "name", _name, NAME_LEN);
}
