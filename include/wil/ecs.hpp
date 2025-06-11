#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <set>

namespace wil {

constexpr size_t MAX_COMPONENTS = 64;

using Entity		= uint32_t;
using ComponentType = uint8_t;
using Signature		= std::bitset<MAX_COMPONENTS>;

class BaseComponentArray
{
public:

	BaseComponentArray(ComponentType type) : type_(type) {}

	virtual ~BaseComponentArray() = default;

	virtual void EntityDestroyed(Entity entity) = 0;

	ComponentType GetType() const { return type_; }

private:
	ComponentType type_;
};

template<typename T>
class ComponentArray : public BaseComponentArray
{
public:

	ComponentArray(ComponentType type) : BaseComponentArray(type) {};

	void InsertData(Entity entity, T&& component)
	{
		entity_to_index_[entity] = components_.size();
		components_.emplace_back(std::forward<T>(component));
		index_to_entity.emplace_back(entity);
	}

	void RemoveData(Entity entity)
	{
		size_t i = entity_to_index_.at(entity);
		size_t last = components_.size() - 1;

		components_[i] = components_[last];
		entity_to_index_.at(index_to_entity[last]) = i;
		index_to_entity[i] = index_to_entity[last];

		components_.pop_back();
		entity_to_index_.erase(entity);
		index_to_entity.pop_back();
	}

	T& GetData(Entity entity)
	{
		return components_[entity_to_index_.at(entity)];
	}

	void EntityDestroyed(Entity entity) override
	{
		if (entity_to_index_.count(entity))
			RemoveData(entity);
	}

private:

	std::vector<T> components_;
	std::unordered_map<Entity, size_t> entity_to_index_;
	std::vector<Entity> index_to_entity;
};

struct EntityView
{
	Signature pass;
	std::set<Entity> set;
};

class System
{
public:

	System(class Registry &registry) {}

	virtual ~System() = default;
};

class Registry
{
public:

	Registry(size_t entities_reserve = 1000)
	{
		signatures_.reserve(entities_reserve);
	}

	// Entity methods
	Entity CreateEntity()
	{
		++entities_count;

		if (refill_entities_.empty())
		{
			Entity e = signatures_.size();
			signatures_.push_back(0);
			return e;
		}
		else
		{
			Entity e = refill_entities_.back();
			refill_entities_.pop_back();
			return e;
		}
	}

	void DestroyEntity(Entity entity)
	{
		signatures_[entity].reset();
		refill_entities_.push_back(entity);
		--entities_count;

		for (auto const& [_, component] : component_arrays_)
			component->EntityDestroyed(entity);

		for (auto const& view : entity_views_)
			view->set.erase(entity);
	}


	// Component methods
	template<typename T>
	void RegisterComponent()
	{
		auto i = std::type_index(typeid(T));
		if (!component_arrays_.count(i))
			component_arrays_[i]
				= std::make_unique<ComponentArray<T>>(component_arrays_.size());
	}

	template<class... Ts>
	void AddComponents(Entity entity, Ts&&... components)
	{
		(RegisterComponent<Ts>(), ...);
		(GetComponentArray<Ts>().InsertData(entity, std::forward<Ts>(components)), ...);

		auto signature = signatures_[entity];
		(signature.set(GetComponentType<Ts>(), true), ...);
		signatures_[entity] = signature;

		for (auto view : entity_views_)
		{
			if ((signature & view->pass) == view->pass)
				view->set.insert(entity);
			else
				view->set.erase(entity);
		}
	}

	template<class... Ts>
	void RemoveComponents(Entity entity)
	{
		(GetComponentArray<Ts>().RemoveData(entity), ...);

		auto signature = signatures_[entity];
		(signature.set(GetComponentType<Ts>(), false), ...);
		signatures_[entity] = signature;

		for (auto view : entity_views_)
		{
			if ((signature & view->pass) == view->pass)
				view->set.insert(entity);
			else
				view->set.erase(entity);
		}
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return GetComponentArray<T>().GetData(entity);
	}

	template<class... Ts>
	std::tuple<Ts&...> GetComponents(Entity entity)
	{
		return std::tie<Ts&...>(GetComponent<Ts>(entity)...);
	}

	template<class... Ts>
	bool HasComponents(Entity entity)
	{
		return (signatures_[entity].test(GetComponentType<Ts>()) && ...);
	}

	template<typename T>
	ComponentType GetComponentType() const
	{
		return component_arrays_.at(std::type_index(typeid(T)))->GetType();
	}

	template<class... Ts>
	void RegisterEntityView(EntityView &view)
	{
		(RegisterComponent<Ts>(), ...);
		(view.pass.set(GetComponentType<Ts>(), true), ...);
		entity_views_.emplace_back(&view);
	}

	template<typename T>
	T &RegisterSystem()
	{
		auto i = std::type_index(typeid(T));
		systems_[i] = std::make_unique<T>(*this);
		return *static_cast<T*>(systems_.at(i).get());
	}

	template<class T>
	T &GetSystem()
	{
		return systems_.at(std::type_index(typeid(T)));
	}

private:

	template<typename T>
	ComponentArray<T> &GetComponentArray()
	{
		return *static_cast<ComponentArray<T>*>(
				component_arrays_.at(std::type_index(typeid(T))).get());
	}

	// Entity manager
	std::vector<Signature> signatures_;
	std::vector<Entity> refill_entities_;
	size_t entities_count;

	// Component manager
	std::unordered_map<std::type_index, std::unique_ptr<BaseComponentArray>> component_arrays_{};

	// System manager
	std::unordered_map<std::type_index, std::unique_ptr<System>> systems_{};
	std::vector<EntityView*> entity_views_;
};

}
