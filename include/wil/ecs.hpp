#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <set>

namespace wil {

constexpr size_t MAX_COMPONENTS = 32;

using Entity		= uint32_t;
using ComponentType = uint8_t;
using Signature		= std::bitset<MAX_COMPONENTS>;

class EntityManager
{
public:

	EntityManager(size_t reserve_entities) : entities_count(0)
	{
		signatures_.reserve(reserve_entities);
	}

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
	}

	void SetSignature(Entity entity, Signature signature)
	{
		signatures_[entity] = signature;
	}

	Signature GetSignature(Entity entity) const
	{
		return signatures_[entity];
	}

private:

	std::vector<Signature> signatures_;
	std::vector<Entity> refill_entities_;
	size_t entities_count;
};

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

	void InsertData(Entity entity, T component)
	{
		entity_to_index_[entity] = components_.size();
		components_.emplace_back(component);
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

class ComponentManager
{
public:

	template<typename T>
	void RegisterComponent()
	{
		component_arrays_[std::type_index(typeid(T))]
			= std::make_unique<ComponentArray<T>>(component_arrays_.size());
	}

	template<typename T>
	ComponentType GetComponentType() const
	{
		return component_arrays_.at(std::type_index(typeid(T)))->GetType();
	}

	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		GetComponentArray<T>().InsertData(entity, component);
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		GetComponentArray<T>().RemoveData(entity);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return GetComponentArray<T>().GetData(entity);
	}

	void EntityDestroyed(Entity entity)
	{
		for (auto const& [_, component] : component_arrays_)
			component->EntityDestroyed(entity);
	}

private:

	template<typename T>
	ComponentArray<T> &GetComponentArray()
	{
		return *static_cast<ComponentArray<T>*>(
				component_arrays_.at(std::type_index(typeid(T))).get());
	}

	std::unordered_map<std::type_index, std::unique_ptr<BaseComponentArray>> component_arrays_{};
};

class System
{
public:

	System(ComponentManager &cm) : cm_(cm) {}

	virtual ~System() = default;

	virtual Signature GetSignature() const = 0;

	std::set<Entity> &GetEntities() { return entities_; }

protected:

	template<class... Ts>
	Signature MakeSignature() const {
		Signature result;
		(result.set(cm_.GetComponentType<Ts>(), true), ...);
		return result;
	}

private:

	ComponentManager &cm_;
	std::set<Entity> entities_;
};

class SystemManager
{
public:

	template<typename T>
	T &RegisterSystem(ComponentManager &cm)
	{
		auto i = std::type_index(typeid(T));
		systems_[i] = std::make_unique<T>(cm);
		return *static_cast<T*>(systems_.at(i).get());
	}

	void EntityDestroyed(Entity entity)
	{
		for (auto const& [_, sys] : systems_)
			sys->GetEntities().erase(entity);
	}

	void EntitySignatureChanged(Entity entity, Signature entitySignature)
	{
		for (auto const& [_, sys] : systems_)
		{
			if ((entitySignature & sys->GetSignature()) == sys->GetSignature())
				sys->GetEntities().insert(entity);
			else
				sys->GetEntities().erase(entity);
		}
	}

private:

	std::unordered_map<std::type_index, std::unique_ptr<System>> systems_{};
};

class Registry
{
public:

	Registry(size_t entities_reserve = 1000)
	{
		entity_manager_ = std::make_unique<EntityManager>(entities_reserve);
		component_manager_ = std::make_unique<ComponentManager>();
		system_manager_ = std::make_unique<SystemManager>();
	}

	// Entity methods
	Entity CreateEntity()
	{
		return entity_manager_->CreateEntity();
	}

	void DestroyEntity(Entity entity)
	{
		entity_manager_->DestroyEntity(entity);

		component_manager_->EntityDestroyed(entity);

		system_manager_->EntityDestroyed(entity);
	}


	// Component methods
	template<typename T>
	void RegisterComponent()
	{
		component_manager_->RegisterComponent<T>();
	}

	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		component_manager_->AddComponent<T>(entity, component);

		auto signature = entity_manager_->GetSignature(entity);
		signature.set(component_manager_->GetComponentType<T>(), true);
		entity_manager_->SetSignature(entity, signature);

		system_manager_->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		component_manager_->RemoveComponent<T>(entity);

		auto signature = entity_manager_->GetSignature(entity);
		signature.set(component_manager_->GetComponentType<T>(), false);
		entity_manager_->SetSignature(entity, signature);

		system_manager_->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return component_manager_->GetComponent<T>(entity);
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		return component_manager_->GetComponentType<T>();
	}

	template<typename T>
	T &RegisterSystem()
	{
		return system_manager_->RegisterSystem<T>(*component_manager_);
	}

private:

	std::unique_ptr<ComponentManager> component_manager_;
	std::unique_ptr<EntityManager> entity_manager_;
	std::unique_ptr<SystemManager> system_manager_;
};

}
