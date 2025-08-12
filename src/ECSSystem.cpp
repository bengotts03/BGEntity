#include "ECSSystem.h"

namespace BGEntity {
    void ECSSystem::AddEntityToSystem(Entity entity) {
        _entitiesInSystem.insert(entity);
    }

    void ECSSystem::RemoveEntityFromSystem(Entity entity) {
        _entitiesInSystem.erase(entity);
    }

    void ECSSystem::ClearSystemEntities() {
        _entitiesInSystem.clear();
    }
}
