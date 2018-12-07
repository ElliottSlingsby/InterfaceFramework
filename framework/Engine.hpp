#include <cstdint>
#include <cassert>
#include <vector>
#include <type_traits>
#include <algorithm>

#include "Utility.hpp"
#include "ObjectPool.hpp"
#include "TypeMask.hpp"

#define MAX_SYSTEMS 8
#define MAX_COMPONENTS 8

#define CHUNK_SIZE 1024 * 1024 * 128 // 1024 kb per ObjectPool chunk

#define ENGINE_MEMBER_NAME _engine
#define ID_MEMBER_NAME _id

#define SUBSCRIBERS (MAX_SYSTEMS > MAX_COMPONENTS ? MAX_SYSTEMS :  MAX_COMPONENTS)

// Helper macros, slightly cleaner syntax

#define INTERFACE_FUNC(EngineType, interfaceFunc) EngineType::InterfaceFunction<decltype(&interfaceFunc), &interfaceFunc>

#define INTERFACE_ENABLE(engine, interfaceFunc) engine.subscribe<std::remove_reference<decltype(*this)>::type, INTERFACE_FUNC(std::remove_reference<decltype(engine)>::type, interfaceFunc)>

#define CALL_SYSTEMS(engine, interfaceFunc) engine.callSystems<INTERFACE_FUNC(std::remove_reference<decltype(engine)>::type, interfaceFunc)>

#define CALL_COMPONENTS(engine, interfaceFunc) engine.callComponents<INTERFACE_FUNC(std::remove_reference<decltype(engine)>::type, interfaceFunc)>

/*
Usage:
	class SystemInterface;
	class ComponentInterface;

	using Engine = InterfaceEngine<SystemInterface, ComponentInterface>;

	class SystemInterface : public Engine::BaseSystem {
		// System scope definitions
	}

	class ComponentInterface : public Engine::BaseComponent {
		// Component scope definitions
	}
*/
template <typename SystemInterface, typename ComponentInterface>
class InterfaceEngine {
public:
	using TypeMask = TypeMask<MAX_COMPONENTS, ComponentInterface>;

	// Destructors for Systems are called virtually
	class BaseSystem {
	protected:
		//InterfaceEngine& ENGINE_MEMBER_NAME;

	public:
		//BaseSystem(InterfaceEngine& engine) : _engine(engine) { }
		virtual ~BaseSystem() { }
	};

	// Destructors for Components are called virtually by their TypePools (no need for virtual destructor here)
	class BaseComponent { 
	protected:
		InterfaceEngine& ENGINE_MEMBER_NAME;
		const uint64_t ID_MEMBER_NAME;

	public:
		BaseComponent(InterfaceEngine& engine, uint64_t id) : _engine(engine), _id(id) { }
	};

	/*
	SystemInterface / ComponenentInterface member functions have static state to store call order of child types, 

	Static state for defining call order for System / Component interface functions.
	i.e. Window might want to update before everything else (for input events), and lateUpdate after everything else (to flip buffer).

	Usage:
		InterfaceFunction<decltype(&SystemInterface::myFunc), &SystemInterface::myFunc>;
	and:
		engine.subscribe<Renderer, INTERFACE_FUNC(Engine, SystemInterface::update)>(1);
		engine.call<INTERFACE_FUNC(Engine, SystemInterface::update)>(0.5);

	(when vs17 supports template <auto> it'd just be):
		engine.subscribe<Renderer, &SystemInterface::update>(1);
		engine.call<&SystemInterface::update>(0.5);
	*/
	template <typename T, T>
	class InterfaceFunction;

	template <typename T, typename ...Ts, void(T::*func)(Ts...)>
	class InterfaceFunction<void(T::*)(Ts...), func> {
		static_assert(std::is_same<T, SystemInterface>::value || std::is_same<T, ComponentInterface>::value);

		struct Subscription {
			uint32_t index;
			int32_t priority = 0;

			inline bool operator<(const Subscription& other) {
				return priority < other.priority;
			}
		};

		using Interface = T;

		static constexpr void(T::*_funcPtr)(Ts...) = func;

		static Subscription _subscribers[SUBSCRIBERS];
		static uint32_t _subscriberCount;

		static inline void _enable(uint32_t index, int32_t priority) {
			auto iter = std::find_if(_subscribers, _subscribers + _subscriberCount, [&](const Subscription& subscriber) {
				return index == subscriber.index;
			});

			if (iter != _subscribers + _subscriberCount) {
				if (iter->priority == priority)
					return;

				iter->priority = priority;
			}
			else {
				_subscribers[_subscriberCount] = { index, priority };
				_subscriberCount++;
			}

			if (_subscriberCount > 1)
				std::sort(_subscribers, _subscribers + _subscriberCount);
		}

		static inline void _disable(uint32_t index) {
			auto iter = std::find_if(_subscribers, _subscribers + _subscriberCount, [&](const Subscription& subscriber) {
				return index == subscriber.index;
			});

			if (iter == _subscribers + _subscriberCount)
				return;

			*iter = _subscribers[_subscriberCount - 1];
			_subscriberCount--;

			if (_subscriberCount > 1)
				std::sort(_subscribers, _subscribers + _subscriberCount);
		}

		friend class InterfaceEngine;
	};

	/*
	Referenced wrapper for entity ID, used when calling iterate. (somewhat obsolete, REMOVE THIS!)
	Calls engine.reference(id) during Constructor(), and calls engine.dereference(id) during ~Destructor().

	Usage:
		engine.iterate([&](Entity& entity){
			if (!entity.has<Transform, Model>())
				return;

			// do stuff
		});
	*/
	class Entity {
		InterfaceEngine& _engine;
		uint64_t _id = 0;

	public:
		inline Entity(InterfaceEngine& engine) : _engine(engine) { }

		inline uint64_t id() const {
			return _id;
		}

		inline void create() {
			if (_id)
				invalidate();

			_id = _engine.createEntity();
			_engine.referenceEntity(_id);
		}

		inline void destroy() {
			assert(_id);

			if (!_id)
				return;

			_engine.destroyEntity(_id);
		}

		inline bool valid() const {
			return _id;
		}

		inline void invalidate() {
			assert(_id);

			if (!_id)
				return;

			_engine.dereferenceEntity(_id);
			_id = 0;
		}

		template <typename T, typename ...Ts>
		inline T* add(Ts&&... args) {
			assert(_id);

			if (!_id)
				return nullptr;

			return _engine.addComponent<T>(_id, std::forward<Ts>(args)...);
		}

		template <typename T>
		inline T* get() {
			assert(_id);

			if (!_id)
				return nullptr;

			return _engine.getComponent<T>(_id);
		}

		template <typename T>
		inline const T* get() const {
			assert(_id);

			if (!_id)
				return nullptr;

			return _engine.getComponent<T>(_id);
		}

		template <typename T>
		inline void remove() {
			assert(_id);

			if (!_id)
				return;

			_engine.removeComponent<T>(_id);
		}

		template <typename ...Ts>
		inline bool has() const {
			assert(_id);

			if (!_id)
				return false;

			return _engine.hasComponents<Ts...>(_id);
		}

		inline void set(uint64_t id) {
			if (_id)
				invalidate();

			if (!_engine.validEntity(id))
				return;

			_id = id;
			_engine.referenceEntity(_id);
		}

		template <typename InterfaceFunction, typename ...Ts>
		void inline call(Ts&&... args) {
			assert(_id);

			if (!_id)
				return;

			_engine.callComponents<InterfaceFunction>(_id, std::forward<Ts>(args)...);
		}

		inline operator uint64_t() const {
			return _id;
		}
	};

private:
	struct Identity {
		enum Flags {
			None = 0,
			Active = 1,
			Destroyed = 2,
			Buffered = 4
		};

		uint32_t version = 0;
		TypeMask mask;
		uint32_t references = 0;
		uint8_t flags = None;
	};

	SystemInterface* _systems[MAX_SYSTEMS] = { nullptr };
	BasePool* _componentPools[MAX_COMPONENTS] = { nullptr };

	std::vector<Identity> _indexIdentities;
	std::vector<uint32_t> _freeIndexes;

	bool _running = true;

	std::vector<uint32_t> _bufferedIndexes;
	bool _iterating = false;

	template <typename T>
	static inline uint32_t _interfaceIndex() {
		static_assert(std::is_base_of<SystemInterface, T>::value || std::is_base_of<ComponentInterface, T>::value);

		uint32_t index;

		if constexpr (std::is_base_of<SystemInterface, T>::value) {
			index = typeIndex<SystemInterface, T>();
			assert(index < MAX_SYSTEMS);
		}
		else {
			index = TypeMask::index<T>();
			assert(index < MAX_COMPONENTS);
		}

		return index;
	}

	inline bool _validIndex(uint32_t index) const {
		if (index >= _indexIdentities.size() || !(_indexIdentities[index].flags & Identity::Active))
			return false;

		return true;
	}

	inline bool _validId(uint64_t id, uint32_t* index, uint32_t* version) const {
		assert(index && version); // sanity

		if (!id)
			return false;

		*index = front64(id);
		*version = back64(id);

		if (!_validIndex(*index) || !(_indexIdentities[*index].flags & Identity::Active))
			return false;

		return true;
	}

	template <typename T>
	inline BasePool* _createPool() {
		static_assert(std::is_base_of<ComponentInterface, T>::value);

		static const uint32_t componentIndex = _interfaceIndex<T>();

		if (!_componentPools[componentIndex])
			_componentPools[componentIndex] = new ObjectPool<T>(CHUNK_SIZE);

		return _componentPools[componentIndex];
	}

	inline void _destroy(uint32_t index) {
		assert(_indexIdentities[index].flags & Identity::Active); // sanity

		if (_indexIdentities[index].references) {
			_indexIdentities[index].flags |= Identity::Destroyed;
			return;
		}

		// remove maxComponents from each pool
		for (uint32_t i = 0; i < MAX_COMPONENTS; i++) {
			if (_indexIdentities[index].mask.has(i)) {
				assert(_componentPools[i]); // sanity
				_componentPools[i]->erase(index);
				_indexIdentities[index].mask.sub(i);
			}
		}

		_indexIdentities[index].flags = Identity::None;
		
		_freeIndexes.push_back(index);
	}

	template <typename Lambda>
	inline void _iterate(uint32_t index, const Lambda& lambda) {
		Identity& identity = _indexIdentities[index];
			
		if (!(identity.flags & Identity::Active) ||
			identity.flags & Identity::Buffered ||
			identity.flags & Identity::Destroyed)
			return;
		
		Entity entity(*this);
		entity.set(combine32(index, identity.version));
		
		lambda(entity);
	}

	template <typename ...Ts>
	inline bool _hasComponents(uint32_t index) const {
		assert(_validIndex(index)); // sanity

		return _indexIdentities[index].mask.has<Ts...>();
	}

	template <uint32_t I, typename Tuple>
	inline typename std::enable_if<I == std::tuple_size<Tuple>::value>::type _registerComponentRecursive() { }

	template <uint32_t I, typename Tuple>
	inline typename std::enable_if<I < std::tuple_size<Tuple>::value>::type _registerComponentRecursive() {
		using T = std::tuple_element<I, Tuple>::type;

		static_assert(std::is_base_of<ComponentInterface, T>::value);

		_createPool<T>();

		_registerComponentRecursive<I + 1, Tuple>();
	}

public:
	inline ~InterfaceEngine() {
		// delete components
		for (uint32_t y = 0; y < _indexIdentities.size(); y++) {
			const Identity& entity = _indexIdentities[y];

			if (!(entity.flags & Identity::Active))
				continue;

			for (uint32_t x = 0; x < MAX_COMPONENTS; x++) {
				if (entity.mask.has(x))
					_componentPools[x]->erase(y);
			}
		}

		// delete component pools
		for (uint32_t i = 0; i < MAX_COMPONENTS; i++) {
			if (_componentPools[i])
				delete _componentPools[i];
		}

		// delete systems
		for (uint32_t i = 0; i < MAX_SYSTEMS; i++) {
			if (_systems[i])
				delete _systems[i];
		}
	}

	template <typename T, typename ...Ts>
	inline void registerSystem(Ts&&... args) {
		static_assert(std::is_base_of<SystemInterface, T>::value);

		const uint32_t index = _interfaceIndex<T>();
		assert(index < MAX_SYSTEMS);

		if (index >= MAX_SYSTEMS)
			return;

		if (_systems[index])
			delete _systems[index];

		_systems[index] = new T(std::forward<Ts>(args)...);
	}

	template <typename ...Ts>
	inline void registerComponents() {
		_registerComponentRecursive<0, std::tuple<Ts...>>();
	}

	inline uint64_t createEntity() {
		uint32_t index;

		if (_freeIndexes.size()) {
			index = *_freeIndexes.begin();
			_freeIndexes.erase(_freeIndexes.begin());
		}
		else {
			assert(_indexIdentities.size() + 1 <= UINT32_MAX);
			index = (uint32_t)_indexIdentities.size();
			_indexIdentities.resize(index + 1);
		}

		if (_iterating) {
			_indexIdentities[index].flags |= Identity::Buffered;
			_bufferedIndexes.push_back(index);
		}

		_indexIdentities[index].flags |= Identity::Active;
		_indexIdentities[index].version++;

		return combine32(index, _indexIdentities[index].version);
	}

	template <typename T, typename ...Ts>
	inline T* addComponent(uint64_t id, Ts&&... args) {
		uint32_t index, version;

		if (!_validId(id, &index, &version))
			return nullptr;

		BasePool* pool = _createPool<T>();

		if (!_hasComponents<T>(index)) {
			_indexIdentities[index].mask.add<T>();

			if constexpr (std::is_constructible<T, Engine&, uint64_t, Ts...>::value)
				pool->insert<T>(index, *this, id, std::forward<Ts>(args)...);
			else
				pool->insert<T>(index, std::forward<Ts>(args)...);
		}

		return (T*)pool->getPtr(index);
	}

	template <typename T, typename InterfaceFunction>
	static inline void subscribe(int32_t priority = 0) {
		static_assert(std::is_base_of<InterfaceFunction::Interface, T>::value);

		const uint32_t index = _interfaceIndex<T>();
		InterfaceFunction::_enable(index, priority);
	}

	template <typename T, typename InterfaceFunction>
	static inline void unsubscribe() {
		static_assert(std::is_base_of<InterfaceFunction::Interface, T>::value);

		const uint32_t index = _interfaceIndex<T>();
		InterfaceFunction::_disable(index, priority);
	}

	template <typename InterfaceFunction, typename ...Ts>
	void inline callSystems(Ts&&... args) {
		static_assert(std::is_same<InterfaceFunction::Interface, SystemInterface>::value);

		for (uint32_t i = 0; i < InterfaceFunction::_subscriberCount; i++)
			(_systems[InterfaceFunction::_subscribers[i].index]->*InterfaceFunction::_funcPtr)(std::forward<Ts>(args)...);
	}

	template <typename InterfaceFunction, typename ...Ts>
	void inline callComponents(uint64_t id, Ts&&... args) {
		static_assert(std::is_same<InterfaceFunction::Interface, ComponentInterface>::value);

		uint32_t index, version;

		if (!_validId(id, &index, &version))
			return;

		const TypeMask& mask = _indexIdentities[index].mask;

		for (uint32_t i = 0; i < InterfaceFunction::_subscriberCount; i++) {
			uint32_t componentIndex = InterfaceFunction::_subscribers[i].index;

			if (!mask.has(componentIndex))
				continue;

			ComponentInterface* componentInterface = (ComponentInterface*)_componentPools[componentIndex]->getPtr(index);
			(componentInterface->*InterfaceFunction::_funcPtr)(std::forward<Ts>(args)...);
		}
	}
	
	template <typename T>
	inline bool hasSystem() const {
		static_assert(std::is_base_of<SystemInterface, T>::value);

		return _systems[_interfaceIndex<T>()];
	}

	template <typename T>
	inline T& system() {
		static_assert(std::is_base_of<SystemInterface, T>::value);

		uint32_t systemIndex = _interfaceIndex<T>();

		assert(_systems[systemIndex]);

		return *(T*)_systems[systemIndex];
	}

	inline bool running() const {
		return _running;
	}

	inline void quit() {
		_running = false;
	}

	inline bool validEntity(uint64_t id) {
		uint32_t index, version;
		return _validId(id, &index, &version);
	}

	inline void destroyEntity(uint64_t id) {
		uint32_t index, version;
		
		if (!_validId(id, &index, &version))
			return;

		_destroy(index);
	}

	template <typename T>
	inline T* getComponent(uint64_t id) {
		return const_cast<T*>(std::as_const(*this).getComponent<T>(id));
	}

	template <typename T>
	inline const T* getComponent(uint64_t id) const {
		uint32_t index, version;

		if (!_validId(id, &index, &version) || !_hasComponents<T>(index))
			return nullptr;

		return (T*)_componentPools[_interfaceIndex<T>()]->getPtr(index);
	}

	template <typename T>
	inline void removeComponent(uint64_t id) {
		uint32_t index, version;

		if (!_validId(id, &index, &version) || !_hasComponents<T>(index))
			return nullptr;

		_componentPools[_interfaceIndex<T>()]->erase(index);
		_indexIdentities[index].mask.sub<T>();
	}

	template <typename ...Ts>
	inline bool hasComponents(uint64_t id) const {
		uint32_t index, version;

		if (!_validId(id, &index, &version) || !_hasComponents<Ts...>(index))
			return false;

		return true;
	}

	inline void referenceEntity(uint64_t id) {
		uint32_t index, version;

		if (!_validId(id, &index, &version))
			return;

		_indexIdentities[index].references++;
	}

	inline void dereferenceEntity(uint64_t id) {
		uint32_t index, version;

		if (!_validId(id, &index, &version))
			return;

		if (!_indexIdentities[index].references)
			return;

		_indexIdentities[index].references--;

		if (_indexIdentities[index].references == 0 && _indexIdentities[index].flags & Identity::Destroyed)
			_destroy(index);
	}

	inline uint32_t entityCount() const {
		return (_indexIdentities.size() + _bufferedIndexes.size()) - _freeIndexes.size();
	}

	template <typename Lambda>
	inline void iterateEntities(const Lambda& lambda) {
		_iterating = true;

		for (uint32_t i = 0; i < _indexIdentities.size(); i++)
			_iterate(i, lambda);
		
		while (_bufferedIndexes.size()) {
			uint32_t index = _bufferedIndexes[0];
			_bufferedIndexes.erase(_bufferedIndexes.begin());
		
			_indexIdentities[index].flags &= ~Identity::Buffered;
			_iterate(index, lambda);
		}

		_iterating = false;
	}

	bool getEntityState(uint64_t id, uint32_t* index, TypeMask* mask) const {
		assert(index && mask);

		uint32_t version;

		if (!_validId(id, index, &version))
			return false;

		*mask = _indexIdentities[*index].mask;

		return true;
	}

	uint64_t setEntityState(uint32_t index, const TypeMask& mask) {
		assert(!_validIndex(index)); // can't be valid index

		if (_indexIdentities.size() >= index)
			_indexIdentities.resize(index + 1);

		_indexIdentities[index].version++;
		_indexIdentities[index].flags |= Identity::Active;
		_indexIdentities[index].mask = mask;

		uint64_t id = combine32(index, _indexIdentities[index].version);

		for (uint32_t i = 0; i < MAX_COMPONENTS; i++) {
			if (!mask.has(i))
				continue;

			assert(_componentPools[i]); // component must already be registered

			_componentPools[i]->insert<ComponentInterface>(index, *this, id);
		}

		return id;
	}
};

template <typename SystemInterface, typename ComponentInterface>
template <typename T, typename ...Ts, void(T::*func)(Ts...)>
typename InterfaceEngine<SystemInterface, ComponentInterface>::InterfaceFunction<void(T::*)(Ts...), func>::Subscription InterfaceEngine<SystemInterface, ComponentInterface>::InterfaceFunction<void(T::*)(Ts...), func>::_subscribers[SUBSCRIBERS] = { 0 };

template <typename SystemInterface, typename ComponentInterface>
template <typename T, typename ...Ts, void(T::*func)(Ts...)>
uint32_t InterfaceEngine<SystemInterface, ComponentInterface>::InterfaceFunction<void(T::*)(Ts...), func>::_subscriberCount = 0;