#pragma once

#include <bitset>
#include <queue>
#include <set>
#include <vector>

namespace BGEntity {
    using Entity = std::uint32_t;
    // constexpr BGEntity::Entity MAX_ENTITIES = std::numeric_limits<Entity>::max();
    constexpr BGEntity::Entity MAX_ENTITIES = 10000;

    constexpr size_t MAX_COMPONENTS = 64;
    using ComponentSet = std::bitset<MAX_COMPONENTS>;
    using ComponentName = const char*;
    using ComponentID = uint32_t;

    template<typename ...Components>
    class ECSView;

    class ISparseSet {
    public:
        virtual ~ISparseSet() = default;
        virtual size_t GetSize() = 0;
        virtual bool HasEntity(Entity entity) = 0;
        virtual std::vector<Entity> GetEntities() = 0;
        virtual void OnEntityDestroyed(Entity entity) = 0;
    };
    template<typename T>
    class SparseSet : public ISparseSet{
    public:
        SparseSet() {
            _denseComponents.reserve(1000);
            _denseIndexToEntity.reserve(1000);
            _entityToDenseIndex.reserve(1000);
        }
        ~SparseSet() override = default;

        void Add(const Entity entity, T component) {
            size_t denseIndex = _size;

            // Sparse: Entity -> Dense Index
            _entityToDenseIndex[entity] = denseIndex;
            // Reverse: Dense Index -> Entity
            _denseIndexToEntity[denseIndex] = entity;

            if (denseIndex >= _denseComponents.size())
                _denseComponents.push_back(component);
            else
                _denseComponents[denseIndex] = component;

            _size++;
        }

        void Remove(const Entity entity) {
            size_t indexToRemoveFromDense = _entityToDenseIndex[entity];
            size_t lastIndex = _size - 1;

            if (indexToRemoveFromDense != lastIndex) {
                _denseComponents[indexToRemoveFromDense] = _denseComponents[lastIndex];

                Entity entityToMove = _denseIndexToEntity[lastIndex];
                _entityToDenseIndex[entityToMove] = indexToRemoveFromDense;
                _denseIndexToEntity[indexToRemoveFromDense] = entityToMove;
            }

            _entityToDenseIndex.erase(entity);
            _denseIndexToEntity.erase(lastIndex);

            _size--;
        }

        T& Get(Entity entity) {
            return _denseComponents[_entityToDenseIndex.at(entity)];
        }

        void OnEntityDestroyed(const Entity entity) override {
            if (_entityToDenseIndex.contains(entity))
                Remove(entity);
        }

        bool HasEntity(Entity entity) override {
            if (_entityToDenseIndex.contains(entity))
                return true;

            return false;
        }

        std::vector<Entity> GetEntities() override{
            std::vector<Entity> entities;
            entities.reserve(_denseIndexToEntity.size());

            int ind = 0;
            for (auto [dense, entity]: _denseIndexToEntity) {
                entities.emplace(entities.begin() + ind, entity);
                ind++;
            }

            return entities;
        }

        size_t GetSize() override {
            return _size;
        }
    private:
        std::vector<T> _denseComponents;

        std::unordered_map<Entity, size_t> _entityToDenseIndex;
        std::unordered_map<size_t, Entity> _denseIndexToEntity;

        size_t _size = 0;
    };

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
        ECS() {
            _availableEntities = std::queue<Entity>();
            for (int i = 0; i < MAX_ENTITIES; ++i) {
                _availableEntities.push(i);
            }
        }
        ~ECS() = default;

        Entity CreateEntity() {
            Entity e = _availableEntities.front();
            _availableEntities.pop();

            _livingEntities.emplace_back(e);

            return e;
        }
        void DeleteEntity(Entity entity) {
            OnDestroyEntity(entity);

            _livingEntities.erase(_livingEntities.begin() + entity);
            _availableEntities.push(entity);
        }
        void OnDestroyEntity(Entity entity) const {
            for (auto& pair : _componentPools) {
                auto& component = pair.second;

                component->OnEntityDestroyed(entity);
            }
        }

        void SetComponentSet(Entity entity, ComponentSet componentSet) {
            _entityComponentSets[entity] = componentSet;
        }
        ComponentSet GetComponentSet(Entity entity) const {
            return _entityComponentSets[entity];
        }

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
