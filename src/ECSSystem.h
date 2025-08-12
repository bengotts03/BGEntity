#pragma once

#include <set>
#include "ECS.h"

namespace BGEntity {
    class ECSSystem {
    public:
        void AddEntityToSystem(Entity entity);
        void RemoveEntityFromSystem(Entity entity);
        void ClearSystemEntities();
    protected:
        std::set<Entity> _entitiesInSystem;
    };
}
