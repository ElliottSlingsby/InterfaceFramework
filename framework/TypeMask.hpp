#pragma once

#include "Utility.hpp"

#include <cstdint>
#include <bitset>
#include <type_traits>
#include <cassert>

template <size_t width>
class TypeMask {
	std::bitset<width> _mask;

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if<i == sizeof...(Ts), void>::type _fill(bool value);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if < i < sizeof...(Ts), void>::type _fill(bool value);

public:
	inline TypeMask<width>& operator=(const TypeMask<width>& other);

	template <typename T>
	inline static uint32_t index();

	template <typename ...Ts>
	inline void fill();

	template <typename ...Ts>
	inline void add();

	template <typename ...Ts>
	inline void sub();

	inline void add(uint32_t i);

	inline void sub(uint32_t i);

	template <typename ...Ts>
	inline bool has() const;

	inline bool has(uint32_t i) const;

	inline bool empty() const;

	inline void clear();

	template <typename ...Ts>
	inline static TypeMask<width> create();
};

template <size_t width>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i == sizeof...(Ts), void>::type TypeMask<width>::_fill(bool value) { }

template <size_t width>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i < sizeof...(Ts), void>::type TypeMask<width>::_fill(bool value) {
	_fill<i + 1, Ts...>(value);

	using T = typename std::tuple_element<i, std::tuple<Ts...>>::type;
	_mask.set(typeIndex<TypeMask, T>(), value);
}

template <size_t width>
TypeMask<width>& TypeMask<width>::operator=(const TypeMask<width>& other) {
	_mask = other._mask;
	return *this;
}

template<size_t width>
template<typename T>
inline uint32_t TypeMask<width>::index(){
	return typeIndex<TypeMask, T>();
}

template <size_t width>
template <typename ...Ts>
void TypeMask<width>::fill() {
	_mask.reset();
	_fill<0, Ts...>(true);
}

template <size_t width>
template <typename ...Ts>
void TypeMask<width>::add() {
	_fill<0, Ts...>(true);
}

template <size_t width>
template <typename ...Ts>
void TypeMask<width>::sub() {
	_fill<0, Ts...>(false);
}

template <size_t width>
template <typename ...Ts>
bool TypeMask<width>::has() const {
	TypeMask<width> other = create<Ts...>();

	unsigned long check = other._mask.to_ulong();
	return (_mask.to_ulong() & check) == check;
}

template<size_t width>
inline void TypeMask<width>::add(uint32_t i){
	if (i >= width)
		return;

	_mask[i] = true;
}

template<size_t width>
inline void TypeMask<width>::sub(uint32_t i){
	if (i >= width)
		return;

	_mask[i] = false;
}

template<size_t width>
bool TypeMask<width>::has(uint32_t i) const {
	return _mask[i];
}

template <size_t width>
bool TypeMask<width>::empty() const {
	return _mask.to_ulong() == 0;
}

template<size_t width>
void TypeMask<width>::clear() {
	_mask = 0;
}

template <size_t width>
template <typename ...Ts>
TypeMask<width> TypeMask<width>::create() {
	TypeMask<width> mask;
	mask.fill<Ts...>();

	return mask;
}