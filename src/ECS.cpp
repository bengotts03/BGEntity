#include "ECS.h"
#include "SparseSet.h"

namespace BGEntity {
    ECS::ECS(){
        _availableEntities = std::queue<Entity>();
        for (int i = 0; i < MAX_ENTITIES; ++i) {
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
    }

    void ECS::SetComponentSet(Entity entity, ComponentSet componentSet) {
        _entityComponentSets[entity] = componentSet;
    }

    ComponentSet ECS::GetComponentSet(Entity entity) {
        return _entityComponentSets[entity];
    }
}
