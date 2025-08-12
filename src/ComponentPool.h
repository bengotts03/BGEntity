#pragma once

#include <vector>
#include "ECS.h"
#include "IComponentPool.h"

namespace BGEntity {
    template<typename T>
    class ComponentPool : public IComponentPool{
    public:
        void Add(const Entity entity, T component) {
            size_t newIndex = _size;

            _entityToComponentIndexMap[entity] = newIndex;
            _componentIndexToEntityMap[newIndex] = entity;
            _components[newIndex] = component;

            _size++;
        }

        void Remove(const Entity entity) {
            size_t removedIndex = _entityToComponentIndexMap[entity];
            size_t indexOfLast = _size - 1;

            Entity entityToMove = _componentIndexToEntityMap[indexOfLast];

            _entityToComponentIndexMap[entityToMove] = removedIndex;
            _componentIndexToEntityMap[removedIndex] = entityToMove;

            _entityToComponentIndexMap.erase(indexOfLast);
            _componentIndexToEntityMap.erase(indexOfLast);

            _size--;
        }

        T& Get(Entity entity) {
            return _components[entity];
        }

        void OnEntityDestroyed(const Entity entity) {
            if (_entityToComponentIndexMap.contains(entity))
                Remove(entity);
        }
    private:
        const int MAX_COMPONENTS;
        std::array<T, 32> _components;

        std::unordered_map<Entity, size_t> _entityToComponentIndexMap;
        std::unordered_map<size_t, Entity> _componentIndexToEntityMap;

        size_t _size{};
    };
}
