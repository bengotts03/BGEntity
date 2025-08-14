#pragma once

#include <bitset>
#include <queue>
#include <set>
#include <vector>

#include "ISparseSet.h"
#include "SparseSet.h"

namespace BGEntity {
    using Entity = std::uint32_t;
    constexpr BGEntity::Entity MAX_ENTITIES = 400;

    using ComponentSet = std::bitset<MAX_ENTITIES>;
    using ComponentName = const char*;
    using ComponentID = uint32_t;

    template<typename ...Components>
    class ECSView;

    class ISparseSet;
    template<typename T>
    class SparseSet;

    class ECSSystem;

    class ECS {
    private:
        template<typename ...>
        friend class ECSView;

        std::queue<Entity> _availableEntities;
        std::vector<Entity> _livingEntities;
        // Index of the array represents the entity, the value is a bitset of whether or not the entity has that component
        std::array<ComponentSet, MAX_ENTITIES> _entityComponentSets{};

        std::unordered_map<ComponentName, ComponentID> _componentNameToIDs;
        std::unordered_map<ComponentName, std::unique_ptr<ISparseSet>> _componentPools;
        ComponentID _currentComponentIDIndex = 0;
    public:
        explicit ECS();
        ~ECS();

        Entity CreateEntity();
        void DeleteEntity(Entity entity);
        void OnDestroyEntity(Entity entity);

        void SetComponentSet(Entity entity, ComponentSet componentSet);
        ComponentSet GetComponentSet(Entity entity);

        template<typename Component>
        SparseSet<Component>* GetComponentPool() {
            ISparseSet* pool = _componentPools[GetComponentName<Component>()].get();
            return static_cast<SparseSet<Component>*>(pool);
        }

        #pragma region Component Management
        template<typename Component>
        void RegisterComponent() {
            const char* name = typeid(Component).name();

            _componentNameToIDs.insert({
                name,
                _currentComponentIDIndex
            });
            _componentPools.insert({
                name,
                std::make_unique<SparseSet<Component>>()
            });

            _currentComponentIDIndex++;
        }

        template<typename T>
        void AddComponent(Entity entity, T component) {
            GetComponentPool<T>()->Add(entity, component);

            auto componentSet = GetComponentSet(entity);
            componentSet.set(GetComponentID<T>(), true);
            SetComponentSet(entity, componentSet);
        }
        template<typename T>
        void RemoveComponent(Entity entity) {
            GetComponentPool<T>()->Remove(entity);

            auto componentSet = GetComponentSet(entity);
            componentSet.set(GetComponentID<T>(), false);
            SetComponentSet(entity, componentSet);
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

        template<typename ...Components>
        ECSView<Components...> View() {
            return ECSView<Components...>(this);
        }
    };

    template<typename ...Components>
    class ECSView {
    private:
        ECS* _ecs;

        std::vector<ISparseSet*> _componentSetsInView;
        ISparseSet* _smallestSet;

        [[nodiscard]]
        bool HasAllComponents(Entity entity) const {
            for (auto set: _componentSetsInView) {
                if (!set->HasEntity(entity))
                    return false;
            }

            return true;
        }

        template<typename Func>
        void ForEachImplementation(Func function) {
            for (auto& entity : _smallestSet->GetEntities()) {
                if (HasAllComponents(entity)) {
                    if constexpr (std::is_invocable_v<Func, Entity, Components&...>)
                        function(entity, _ecs->GetComponent<Components>(entity)...);
                    else if constexpr (std::is_invocable_v<Func, Components&...>)
                        function(_ecs->GetComponent<Components>(entity)...);
                    else
                        static_assert(false, "Bad lambda provided to .ForEach(), parameter pack does not match lambda args");
                }
            }
        }
    public:
        ECSView(ECS* ecs) : _componentSetsInView {ecs->GetComponentPool<Components>()...}{
            _ecs = ecs;

            _smallestSet = *std::min_element(_componentSetsInView.begin(), _componentSetsInView.end(), [](ISparseSet* a, ISparseSet* b) {
               return a->GetSize() < b->GetSize();
            });
        }

        void ForEach(std::function<void(Components&...)> function) {
            ForEachImplementation(function);
        }

        void ForEach(std::function<void(Entity, Components&...)> function) {
            ForEachImplementation(function);
        }
    };
}
