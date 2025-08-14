#pragma once

#include <ranges>
#include <vector>

#include "ECS.h"
#include "ISparseSet.h"

namespace BGEntity {
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
            for (auto entity: _denseIndexToEntity | std::views::values) {
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
}
