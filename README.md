# BGEntity ECS System

A lightweight, high-performance Entity Component System (ECS) implementation in C++ designed for game development and real-time applications.

## Features

- ✅ Type-safe component registration and management
- ⚡ Efficient entity creation and component attachment
- 🔍 Flexible view system for querying entities with specific component sets
- 🚀 High-performance iteration over component combinations
- 📊 Minimal overhead for large numbers of entities

## Quick Start

### 1. Initialize the ECS

```cpp
#include "BGEntity/ECS.h"

BGEntity::ECS* ecs = new BGEntity::ECS();
```

### 2. Define Components

Components are simple structs containing data:

```cpp
struct Transform {
    glm::vec2 Position;
    glm::vec2 Scale;

    Transform() : Position(glm::vec2(0, 0)), Scale(glm::vec2(1, 1)) {}
    explicit Transform(glm::vec2 pos, glm::vec2 scale = glm::vec2(1, 1)) 
        : Position(pos), Scale(scale) {}
};

struct Physics {
    glm::vec2 Velocity;
    glm::vec2 AngularVelocity;
};
```

### 3. Register Components

Before using components, they must be registered with the ECS:

```cpp
ecs->RegisterComponent<Transform>();
ecs->RegisterComponent<Physics>();
```

### 4. Create Entities

```cpp
auto entity = ecs->CreateEntity();
ecs->AddComponent<Transform>(entity, Transform({1, 1}));
ecs->AddComponent<Physics>(entity, {{0, 0}, {0, 0}});
```

### 5. Query and Iterate Over Entities

Use views to efficiently query entities with specific component combinations:

```cpp
auto view = ecs->View<Transform, Physics>();

view.ForEach([&](BGEntity::Entity entity, auto& transform, auto& physics) {
    std::cout << "Entity (" << entity << ") at Position: " 
              << transform.Position.x << "," << transform.Position.y << std::endl;
});

OR

view.ForEach([](auto& transform, auto& physics) {
    std::cout << "Entity (" << entity << ") at Position: " 
              << transform.Position.x << "," << transform.Position.y << std::endl;
});

```

## Requirements

- **C++20** or later
- **CMake 3.31+**
- **GLM** (OpenGL Mathematics) for vector operations

entities.ForEach([&](auto& transform, auto& physics) {
    // Update logic for movable entities
});
