#pragma once
#include <glm/glm.hpp>

// Core game types: rect, ball, block, powerUp

// 2D Rect struct
struct Rect {
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec4 color;
};

// a moving ball that collides with blocks, paddle, and walls
// primary object which the player interacts with via the paddle
struct Ball {
    glm::vec2 pos;
    glm::vec2 vel;
    float radius;
    glm::vec4 color;
};

// a static destructible rectangular block
// blocks who become hit by the ball become inactive and may spawn powerups
struct Block {
    Rect rect;
    bool alive = true;
    int row = 0;
    char symbol = '1';
};

// a collectable powerup that falls from destroyed blocks
// Types:
//   PaddleWiden: Increases paddle width temporarily
//   MultiBall: Spawns two additional balls 
//   SlowBall: Reduces ball speed temporarily
//   Piercing: Allows balls to pass through blocks
//   ExtraLife: Grants an additional life 
struct PowerUp {
    enum class Type { PaddleWiden, MultiBall, SlowBall, Piercing, ExtraLife } type = Type::PaddleWiden;
    Rect rect;
    glm::vec2 vel;
    bool active = true; // true if it can be collected, false if it has fallen off screen or been collected
};
