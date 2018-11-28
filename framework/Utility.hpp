#pragma once

#include <cstdint>
#include <chrono>
#include <thread>
#include <utility>
#include <iostream>
#include <string>
#include <fstream>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

inline uint32_t back64(uint64_t i) {
	return static_cast<uint32_t>(i);
}

inline uint32_t front64(uint64_t i) {
	return static_cast<uint32_t>(i >> 32);
}

inline uint64_t combine32(uint32_t front, uint32_t back) {
	return static_cast<uint64_t>(back) + (static_cast<uint64_t>(front) << 32);
}

inline void startTime(TimePoint* point) {
	*point = Clock::now();
}

template <typename T = double>
inline T deltaTime(const TimePoint& point) {
	return std::chrono::duration_cast<std::chrono::duration<T>>(Clock::now() - point).count();
}

template <typename Namespace>
inline uint32_t typeIndexCount(bool add = false) {
	static uint32_t count = 0;

	if (add)
		return count++;

	return count;
}

template <typename Namespace, typename T>
inline uint32_t typeIndex() {
	static uint32_t index = typeIndexCount<Namespace>(true);
	return index;
}

inline std::string replace(char from, char to, std::string text) {
	for (char& c : text) {
		if (c == from)
			c = to;
	}

	return text;
}

inline std::string upperPath(std::string filePath) {
	if (filePath.empty())
		return filePath;

	filePath.pop_back();
	return filePath.substr(0, filePath.find_last_of('/') + 1);
}