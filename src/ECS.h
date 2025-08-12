#pragma once

#include <bitset>
#include <queue>
#include <vector>

#include "IComponentPool.h"
#include "ComponentPool.h"
#include "ECSSystem.h"

namespace BGEntity {
    using Entity = std::uint32_t;
    constexpr Entity MAX_ENTITIES = 32;
    using ComponentSet = std::bitset<MAX_ENTITIES>;

    class IComponentPool;
    template<typename T>
    class ComponentPool;

    class ECSSystem;
    class ECS {
    private:
        const Entity _maxEntities = 0;

        using ComponentName = const char*;
        using ComponentID = uint32_t;

        std::queue<Entity> _availableEntities;
        std::vector<Entity> _livingEntities;
        // Index of the array represents the entity, the value is a bitset of whether or not the entity has that component
        std::array<ComponentSet, MAX_ENTITIES> _entityComponentSets{};

        std::unordered_map<ComponentName, ComponentID> _componentNameToIDs;
        std::unordered_map<ComponentName, std::unique_ptr<IComponentPool>> _componentPools;
        ComponentID _currentComponentIDIndex = 0;

        std::unordered_map<ComponentName, std::shared_ptr<ECSSystem>> _systems;
        // These are the components that the system affects
        std::unordered_map<ComponentName, ComponentSet> _systemComponentSets;
    private:
        template<typename T>
        ComponentPool<T>* GetComponentPool() {
            IComponentPool* pool = _componentPools[GetComponentName<T>()].get();
            return dynamic_cast<ComponentPool<T>*>(pool);
        }
    public:
        explicit ECS(Entity maxEntities);
        ~ECS();

        Entity CreateEntity();
        void DeleteEntity(Entity entity);
        void OnDestroyEntity(Entity entity);

        void SetComponentSet(Entity entity, ComponentSet componentSet);
        ComponentSet GetComponentSet(Entity entity);
        void OnEntityComponentSetChanged(Entity entity, ComponentSet componentSet);

        #pragma region Component Management
        template<typename T>
        void RegisterComponent() {
            const char* name = typeid(T).name();

            _componentNameToIDs.insert({
                name,
                _currentComponentIDIndex
            });
            _componentPools.insert({
                name,
                std::make_unique<ComponentPool<T>>()
            });

            _currentComponentIDIndex++;
        }

        template<typename T>
        void AddComponent(Entity entity, T component) {
            GetComponentPool<T>()->Add(entity, component);

            auto componentSet = GetComponentSet(entity);
            componentSet.set(GetComponentID<T>(), true);
            SetComponentSet(entity, componentSet);

            OnEntityComponentSetChanged(entity, componentSet);
        }
        template<typename T>
        void RemoveComponent(Entity entity) {
            GetComponentPool<T>()->Remove(entity);

            auto componentSet = GetComponentSet(entity);
            componentSet.set(GetComponentID<T>(), false);
            SetComponentSet(entity, componentSet);

            OnEntityComponentSetChanged(entity, componentSet);
        }
        template<typename T>
        ComponentID GetComponentID() {
            const char* name = typeid(T).name();

            return _componentNameToIDs[name];
        }
        template<typename T>
        ComponentName GetComponentName() {
            return typeid(T).name();
        }
        template<typename T>
        T& GetComponent(Entity entity) {
            return GetComponentPool<T>()->Get(entity);
        }

        #pragma endregion

        #pragma region System Management
        template<typename T>
        std::shared_ptr<T> RegisterSystem() {
            auto system = std::make_shared<T>();
            _systems.insert({GetComponentName<T>(), system});

            return system;
        }

        template<typename T>
        void SetSystemComponentSet(ComponentSet set) {
            _systemComponentSets[GetComponentName<T>()] = set;
        }
        #pragma endregion
    };
}
