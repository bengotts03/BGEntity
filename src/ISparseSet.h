#pragma once

#include "ECS.h"

namespace BGEntity {
    using Entity = std::uint32_t;

    class ISparseSet {
    public:
        virtual ~ISparseSet() = default;
        virtual size_t GetSize() = 0;
        virtual bool HasEntity(Entity entity) = 0;
        virtual std::vector<Entity> GetEntities() = 0;
        virtual void OnEntityDestroyed(Entity entity) = 0;
    };
}
