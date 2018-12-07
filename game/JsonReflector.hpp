#pragma once

#include "SystemInterface.hpp"

#include <fstream>

#include <rapidjson\document.h>
#include <rapidjson\prettywriter.h>
#include <rapidjson\stringbuffer.h>

class JsonReflectorOut : public ComponentInterface::BaseReflector {
	rapidjson::Document _document;
	const std::string _path;

	rapidjson::Value _object;

	rapidjson::Value _newString(const std::string& str) {
		rapidjson::Value ref(rapidjson::kStringType);
		ref.SetString(rapidjson::StringRef(str.c_str()), _document.GetAllocator());

		return ref;
	}

	template <typename T>
	void _genericBuffer(const std::string& type, const std::string& property, T* value, uint32_t count = 1) {
		if (!std::is_same<T, char>::value && !count)
			return;

		rapidjson::Value subObject;

		if constexpr (std::is_same<T, char>::value) {
			std::string str;

			if (!count) {
				str = value;
			}
			else {
				str.resize(count);
				std::copy(value, value + count, &str[0]);
			}

			subObject = _newString(str);
		}
		else {
			if (count > 1) {
				subObject = rapidjson::Value(rapidjson::kArrayType);
				subObject.Reserve(count, _document.GetAllocator());

				for (uint32_t i = 0; i < count; i++)
					subObject.PushBack(value[i], _document.GetAllocator());
			}
			else {
				subObject = *value;
			}
		}

		auto name = _newString(type + '.' + property);
		_object.AddMember(name, subObject, _document.GetAllocator());
	}

public:
	JsonReflectorOut(const std::string& path) : BaseReflector(Out), _path(path), _document(rapidjson::kObjectType) { }

	~JsonReflectorOut() {
		close();
	}

	void close() {
		if (!_document.MemberCount())
			return;

		std::fstream file(_path, std::ofstream::out | std::ofstream::trunc);

		if (!file.is_open())
			return;

		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

		writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);

		_document.Accept(writer);

		file << buffer.GetString();
	}

	void processEntity(Engine& engine, uint64_t id) {
		uint32_t index;
		Engine::TypeMask mask;

		engine.getEntityState(id, &index, &mask);

		_object = rapidjson::Value(rapidjson::kObjectType);
		_object.AddMember("mask", _newString(mask.toStr()), _document.GetAllocator());

		CALL_COMPONENTS(engine, ComponentInterface::serialize)(id, *(BaseReflector*)this);

		if (!_object.MemberCount())
			return;

		auto name = _newString(std::to_string(index));
		_document.AddMember(name, _object, _document.GetAllocator());
	}

	bool buffered(const std::string& type, const std::string& property) const final {
		return _object.HasMember(rapidjson::StringRef((type + '.' + property).c_str()));
	}

	void buffer(const std::string& type, const std::string& property, float* value, uint32_t count = 1) final {
		_genericBuffer(type, property, value, count);
	}

	void buffer(const std::string& type, const std::string& property, uint32_t* value, uint32_t count = 1) final {
		_genericBuffer(type, property, value, count);
	}

	void buffer(const std::string& type, const std::string& property, char* value, uint32_t count = 1) final {
		_genericBuffer(type, property, value, count);
	}
};

class JsonReflectorIn : public ComponentInterface::BaseReflector {
	rapidjson::Document _document;
	rapidjson::Value _object;

	template <typename T>
	void _genericBuffer(const std::string& type, const std::string& property, T* value, uint32_t count = 1) {
	
	}

public:
	JsonReflectorIn(const std::string& path) : BaseReflector(In) {
		
	}

	bool buffered(const std::string& type, const std::string& property) const final {
		return _object.HasMember(rapidjson::StringRef((type + '.' + property).c_str()));
	}

	void buffer(const std::string& type, const std::string& property, float* value, uint32_t count = 1) final {
		_genericBuffer(type, property, value, count);
	}

	void buffer(const std::string& type, const std::string& property, uint32_t* value, uint32_t count = 1) final {
		_genericBuffer(type, property, value, count);
	}

	void buffer(const std::string& type, const std::string& property, char* value, uint32_t count = 1) final {
		_genericBuffer(type, property, value, count);
	}
};