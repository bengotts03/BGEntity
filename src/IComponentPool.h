#pragma once

#include "ECS.h"

namespace BGEntity {
    using Entity = std::uint32_t;

    class IComponentPool {
    public:
        virtual ~IComponentPool() = default;
        virtual void OnEntityDestroyed(Entity entity) = 0;
    };
}
