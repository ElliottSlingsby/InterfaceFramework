#pragma once

#include "SystemInterface.hpp"
#include <string>

#define NAME_LEN 32

class Name : public ComponentInterface {
	char _name[NAME_LEN + 1];

public:
	Name(Engine& engine, uint64_t id, const std::string& str = "");

	void set(const std::string& str);
	const char* c_str() const;

	bool operator==(const std::string& str) const;

	void serialize(BaseReflector& reflector) final;
};