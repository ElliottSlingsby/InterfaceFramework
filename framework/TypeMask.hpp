#pragma once

#include "Utility.hpp"

#include <cstdint>
#include <bitset>
#include <type_traits>
#include <cassert>
#include <string>

template <size_t width, typename Base = void>
class TypeMask {
	std::bitset<width> _mask;

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if<i == sizeof...(Ts), void>::type _fill(bool value);

	template <uint32_t i, typename ...Ts>
	inline typename std::enable_if < i < sizeof...(Ts), void>::type _fill(bool value);

public:
	inline TypeMask<width, Base>& operator=(const TypeMask<width, Base>& other);

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
	inline static TypeMask<width, Base> create();

	inline std::string toStr() const;

	inline void fromStr(const std::string& str);
};

template <size_t width, typename Base>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i == sizeof...(Ts), void>::type TypeMask<width, Base>::_fill(bool value) { }

template <size_t width, typename Base>
template <uint32_t i, typename ...Ts>
typename std::enable_if<i < sizeof...(Ts), void>::type TypeMask<width, Base>::_fill(bool value) {
	_fill<i + 1, Ts...>(value);

	using T = typename std::tuple_element<i, std::tuple<Ts...>>::type;

	if constexpr (!std::is_same<Base, void>::value)
		static_assert(std::is_base_of<Base, T>::value);

	_mask.set(typeIndex<TypeMask, T>(), value);
}

template <size_t width, typename Base>
TypeMask<width, Base>& TypeMask<width, Base>::operator=(const TypeMask<width, Base>& other) {
	_mask = other._mask;
	return *this;
}

template <size_t width, typename Base>
template<typename T>
inline uint32_t TypeMask<width, Base>::index(){
	return typeIndex<TypeMask, T>();
}

template <size_t width, typename Base>
template <typename ...Ts>
void TypeMask<width, Base>::fill() {
	_mask.reset();
	_fill<0, Ts...>(true);
}

template <size_t width, typename Base>
template <typename ...Ts>
void TypeMask<width, Base>::add() {
	_fill<0, Ts...>(true);
}

template <size_t width, typename Base>
template <typename ...Ts>
void TypeMask<width, Base>::sub() {
	_fill<0, Ts...>(false);
}

template <size_t width, typename Base>
template <typename ...Ts>
bool TypeMask<width, Base>::has() const {
	TypeMask<width, Base> other = create<Ts...>();

	unsigned long check = other._mask.to_ulong(); // eugh, this probably breaks when width is above 64
	return (_mask.to_ulong() & check) == check;
}

template <size_t width, typename Base>
inline void TypeMask<width, Base>::add(uint32_t i){
	if (i >= width)
		return;

	_mask[i] = true;
}

template <size_t width, typename Base>
inline void TypeMask<width, Base>::sub(uint32_t i){
	if (i >= width)
		return;

	_mask[i] = false;
}

template <size_t width, typename Base>
bool TypeMask<width, Base>::has(uint32_t i) const {
	return _mask[i];
}

template <size_t width, typename Base>
bool TypeMask<width, Base>::empty() const {
	return _mask.to_ulong() == 0;
}

template <size_t width, typename Base>
void TypeMask<width, Base>::clear() {
	_mask = 0;
}

template <size_t width, typename Base>
template <typename ...Ts>
TypeMask<width, Base> TypeMask<width, Base>::create() {
	TypeMask<width, Base> mask;
	mask.fill<Ts...>();

	return mask;
}

template <size_t width, typename Base>

std::string TypeMask<width, Base>::toStr() const {
	std::string str;
	str.resize(width, '0');

	for (uint32_t i = 0; i < width; i++)
		str[i] = (_mask[i] ? '1' : '0');

	return str;
}

template <size_t width, typename Base>
void TypeMask<width, Base>::fromStr(const std::string& str) {
	for (uint32_t i = 0; i < (str.length() > width : width ? str.length()); i++)
		_mask[i] = (str[i] == '1' ? 1 : 0);
}