#pragma once

#include "SystemInterface.hpp"

namespace Test {
	uint64_t createAxis(Engine& engine, const std::string& path, const glm::vec3& position, const glm::quat& rotation);

	void recursivelySetTexture(Engine& engine, uint64_t id, uint32_t textureBufferId);

	uint64_t recursivelyFindName(Engine& engine, uint64_t id, const std::string& name);
};