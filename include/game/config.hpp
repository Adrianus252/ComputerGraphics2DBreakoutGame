#pragma once
#include <glm/glm.hpp>
#include <array>

// central configuration for all Breakout game constants 

namespace BreakoutConfig {

    // Block Configuration
    constexpr int kRows = 11; // number of block rows in each level layout
    constexpr int kCols = 10; // number of block columns in each level layout
    constexpr float kBlockPadding = 8.0f; // space between blocks and screen edges
    constexpr float kBlockTopOffset = 60.0f; //  y offset from top of screen to first block row
    constexpr float kBlockHeight = 24.0f; // Height of each block (width is calculated based on screen size)
    inline const glm::vec4 kBlockGradientBase{0.2f, 0.6f, 0.9f, 1.0f}; // gradient mode base color for blocks (color: blue) 
    inline const glm::vec3 kBlockGradientDelta{0.6f, -0.4f, -0.6f};  // gradient mode color delta from top to bottom rows (color: orange)
    // Dedicated brick color palette used by level symbols ('1'..'9').
    inline const std::array<glm::vec4, 9> kBrickColorPalette{{
        {0.20f, 0.60f, 0.95f, 1.0f}, // 1 - blue
        {0.60f, 0.95f, 0.55f, 1.0f}, // 2 - light green
        {0.62f, 0.35f, 0.90f, 1.0f}, // 3 - purple
        {0.90f, 0.20f, 0.20f, 1.0f}, // 4 - red
        {0.95f, 0.60f, 0.20f, 1.0f}, // 5 - orange
        {0.45f, 0.78f, 0.98f, 1.0f}, // 6 - light blue
        {0.95f, 0.85f, 0.20f, 1.0f}, // 7 - yellow
        {0.25f, 0.80f, 0.35f, 1.0f}, // 8 - green
        {0.55f, 0.35f, 0.20f, 1.0f}  // 9 - brown
    }};

    // Paddle Configuration
    inline const glm::vec2 kPaddleSize{120.0f, 32.0f}; // default paddle dimensions (width x height)
    constexpr float kPaddleBottomOffset = 40.0f; //  y offset from bottom of screen to paddle position
    inline const glm::vec4 kPaddleColor{0.4f, 0.4f, 0.4f, 1.0f}; // normal paddle color (dark gray)
    constexpr float kPaddleWidenScale = 1.5f; // scale multiplier for paddle width (powerup)   
    constexpr float kPaddleWidenTimer = 10.0f; // duration of paddle widen powerup in seconds
    constexpr float kPaddleSpeedBase = 1000.0f; // base paddle speed (multiplied by percentage setting)
    constexpr float kPaddleSpeedPercent = 50.0f; // default paddle speed percentage (50% = 500 pixels/second)

    // Ball Configuration
    constexpr float kBallRadius = 8.0f; // ball radius
    constexpr float kBallSpeed = 320.0f; // normal ball speed  
    constexpr float kBallStickOffset = 2.0f; // small gap between ball and paddle when stuck to paddle
    inline const glm::vec4 kBallColor{0.5f, 0.5f, 0.5f, 1.0f}; // normal ball color (medium grey)

    // Powerup Configuration
    constexpr float kMultiBallAngle = 0.35f;  // Angle offset for multi-ball spread
    constexpr float kPowerupSpawnChance = 0.35f; // 35% for powerup
    constexpr float kPowerupPiercingChance = 0.80f; // 1.0-0.8
    constexpr float kPowerupSlowChance = 0.55f; // 0.8-0.55
    constexpr float kPowerupMultiballChance = 0.40f; //0.55-0.40
    constexpr float kPowerupWidenChance = 0.20f; // 0.4-0.2
    inline const glm::vec2 kPowerupSize{24.0f, 24.0f}; // powerup rectangle size   
    constexpr float kPowerupSpeed = 140.0f; // powerup falling speed
    inline const glm::vec4 kPowerupColorWiden{0.2f, 0.6f, 0.95f, 1.0f}; // PaddleWiden powerup color (blue)
    inline const glm::vec4 kPowerupColorMultiball{0.95f, 0.85f, 0.2f, 1.0f}; // MultiBall powerup color (yellow)
    inline const glm::vec4 kPowerupColorSlow{0.95f, 0.6f, 0.2f, 1.0f}; // SlowBall powerup color (orange)
    inline const glm::vec4 kPowerupColorPiercing{0.9f, 0.1f, 0.1f, 1.0f}; // Piercing powerup color (red)
    inline const glm::vec4 kPowerupColorExtraLife{1.0f, 0.45f, 0.45f, 1.0f}; // ExtraLife powerup color (light red)

    // Timer 
    constexpr float kSlowBallTimer = 8.0f; // duration of slow ball power-up in seconds
    constexpr float kSlowBallMultiplier = 0.6f; // speed multiplier (slow ball powerup)
    constexpr float kPiercingTimer = 15.0f; // Duration of piercing power-up in seconds
   
    // Lives
    constexpr int kLives = 3; // starting number of lives
    constexpr int kMaxLives = 3; // maximum number of lives   

    // Menu Configuration
    constexpr float kMenuWindowWidth = 300.0f; // Width of menu windows in pixels
    constexpr float kMenuWindowHeight = 0.0f; // Height of menu windows (0 = auto-sized)
    constexpr float kPaddleSpeedMinPercent = 0.0f; // Minimum paddle speed percentage in options menu (0%)
    constexpr float kPaddleSpeedMaxPercent = 100.0f; // Maximum paddle speed percentage in options menu (100%)

    // HUD Heart Display Configuration
    constexpr float kHeartPixel = 4.0f;  // size of each pixel in the heart sprite (4x4 pixels)
    constexpr float kHeartPadding = 6.0f;  // padding between individual hearts
    constexpr float kHeartScreenPadding = 24.0f; // padding from screen edge to hearts display
    constexpr int kHeartCols = 11; // number of columns in heart sprite grid (11x7 pixel art)
    constexpr int kHeartRows = 7; // number of rows in heart sprite grid
    inline const glm::vec4 kHeartColor{0.9f, 0.1f, 0.1f, 1.0f}; // normal heart color for base lives (red)

}
