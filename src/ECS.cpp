#include "ECS.h"
#include "ComponentPool.h"

namespace BGEntity {
    ECS::ECS(Entity maxEntities) : _maxEntities(maxEntities){
        _availableEntities = std::queue<Entity>();
        for (int i = 0; i < _maxEntities; ++i) {
            _availableEntities.push(i);
        }
    }

    ECS::~ECS() {
    }

    Entity ECS::CreateEntity() {
        Entity e = _availableEntities.front();
        _availableEntities.pop();

        _livingEntities.emplace_back(e);

        return e;
    }

    void ECS::DeleteEntity(Entity entity) {
        OnDestroyEntity(entity);

        _livingEntities.erase(_livingEntities.begin() + entity);
        _availableEntities.push(entity);
    }

    void ECS::OnDestroyEntity(Entity entity) {
        for (auto& pair : _componentPools) {
            auto& component = pair.second;

            component->OnEntityDestroyed(entity);
        }

        for (auto& pair : _systems) {
            auto& system = pair.second;

            system->RemoveEntityFromSystem(entity);
        }
    }

    void ECS::SetComponentSet(Entity entity, ComponentSet componentSet) {
        _entityComponentSets[entity] = componentSet;
    }

    ComponentSet ECS::GetComponentSet(Entity entity) {
        return _entityComponentSets[entity];
    }

    void ECS::OnEntityComponentSetChanged(Entity entity, ComponentSet entityComponentSet) {
        for (auto& pair : _systems) {
            auto& name = pair.first;
            auto& system = pair.second;
            auto& componentSet = _systemComponentSets[name];

            if ((entityComponentSet & componentSet) == componentSet) {
                system->AddEntityToSystem(entity);
            }
            else {
                system->RemoveEntityFromSystem(entity);
            }
        }
    }
}
