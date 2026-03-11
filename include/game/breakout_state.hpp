#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "game/types.hpp"
#include "game/config.hpp"
#include "game/levels.hpp"

// game state and logic for the breakout game
// gameplay mechanics: ball behavior, collision detection, powerup behavior, paddle behavrior, brick management etc.

struct BreakoutState {
    enum class UpdateResult { None, Win, GameOver };

    glm::vec2 screen{0.0f, 0.0f};
    Rect paddle;
    std::vector<Ball> balls;
    std::vector<Block> blocks;
    std::vector<PowerUp> powerups;
    float ball_speed = BreakoutConfig::kBallSpeed;
    bool ball_stuck = true;
    int lives = BreakoutConfig::kLives;
    int max_lives = BreakoutConfig::kMaxLives;
    float paddle_base_width = BreakoutConfig::kPaddleSize.x;
    float paddle_wide_timer = 0.0f;
    float slow_ball_timer = 0.0f;
    float ball_speed_multiplier = 1.0f;
    float piercing_timer = 0.0f;
    float paddle_speed_base = BreakoutConfig::kPaddleSpeedBase;
    float paddle_speed_percent = BreakoutConfig::kPaddleSpeedPercent;
    bool use_textures = false;
    bool use_gradient_brick_colors = true;
    size_t level_index = 0;
    bool using_custom_level = false;
    std::string custom_level_name;
    std::vector<std::string> custom_level_layout; // storage for custom level loaded from file
    bool powerup_caught_this_frame = false;  // flag to detect powerup catches
    std::mt19937 rng; // random number generator for powerup spawning
    std::uniform_real_distribution<float> dist01{0.0f, 1.0f}; 

    // init game state for a new game session.
    void init(const glm::vec2& screen_in) {
        screen = screen_in; // viewport dimensions
        rng.seed(std::random_device{}()); 
        reset_level();
        reset_ball(true);
    }

    // gets current paddle movement speed in pixels per second.
    float paddle_speed() const {
        return paddle_speed_base * (paddle_speed_percent / 100.0f);
    }

    // determines brick color based on 2 modes
    glm::vec4 block_color_for(int row, char symbol) const {

        // gradient mode: color based on row index (top to bottom)
        if (use_gradient_brick_colors) {
            float t = static_cast<float>(row) / (BreakoutConfig::kRows - 1);
            return {
                BreakoutConfig::kBlockGradientBase.r + BreakoutConfig::kBlockGradientDelta.r * t,
                BreakoutConfig::kBlockGradientBase.g + BreakoutConfig::kBlockGradientDelta.g * t,
                BreakoutConfig::kBlockGradientBase.b + BreakoutConfig::kBlockGradientDelta.b * t,
                BreakoutConfig::kBlockGradientBase.a
            };
        }

        size_t color_index = 0;
        if (symbol >= '1' && symbol <= '9') {
            color_index = static_cast<size_t>(symbol - '1');
        }
        return BreakoutConfig::kBrickColorPalette[color_index];
    }

    // updates color for every block 
    void refresh_block_colors() {
        for (auto& block : blocks) {
            block.rect.color = block_color_for(block.row, block.symbol);
        }
    }

    // when menu change to gradient color mode
    void set_gradient_brick_colors(bool enabled) {
        if (use_gradient_brick_colors == enabled) return;
        use_gradient_brick_colors = enabled;
        refresh_block_colors();
    }

    // resets the current level by clearing blocks and reloading the layout
    // uses custom level layout if using_custom_level is true, otherwise uses normal levels
    // resets paddle and powerups
    void reset_level() {
        blocks.clear();
        const int rows = BreakoutConfig::kRows;
        const int cols = BreakoutConfig::kCols;
        const float padding = BreakoutConfig::kBlockPadding;
        const float top_offset = BreakoutConfig::kBlockTopOffset;
        const float block_width = (screen.x - padding * (cols + 1)) / cols;
        const float block_height = BreakoutConfig::kBlockHeight;

        // build blocks based (custom or normal level)
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                char cell;
                if (using_custom_level && !custom_level_layout.empty()) {
                    cell = custom_level_layout[y][x];
                } else {
                    const auto& layout = BreakoutLevels::layouts[level_index % BreakoutLevels::layouts.size()];
                    cell = layout[y][x];
                }
                
                if (cell == '0') continue;
                Rect rect;
                rect.pos = {
                    padding + x * (block_width + padding),
                    top_offset + y * (block_height + padding)
                };
                rect.size = {block_width, block_height};
                rect.color = block_color_for(y, cell);
                blocks.push_back({rect, true, y, cell});
            }
        }

        // reset paddle position and powerup timers
        paddle.size = BreakoutConfig::kPaddleSize;
        paddle_base_width = paddle.size.x;
        paddle.pos = {screen.x * 0.5f - paddle.size.x * 0.5f, screen.y - BreakoutConfig::kPaddleBottomOffset};
        paddle.color = BreakoutConfig::kPaddleColor;
        powerups.clear();
        paddle_wide_timer = 0.0f;
        slow_ball_timer = 0.0f;
        ball_speed_multiplier = 1.0f;
        piercing_timer = 0.0f;
    }

    // load a custom level from a file
    bool load_custom_level(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        custom_level_layout.clear();
        std::string line;
        int row_count = 0;
        
        // reads custom level layout line by line
        while (std::getline(file, line) && row_count < BreakoutConfig::kRows) {
            // trim line and ensure its 10 characters
            if (line.length() >= BreakoutConfig::kCols) {
                line = line.substr(0, BreakoutConfig::kCols);
            } else {
                // otherwise add zeros, if its to short
                line.append(BreakoutConfig::kCols - line.length(), '0');
            }
           
            // only allow characters '0' to '9', replace invalid chars with '0'
            for (char& ch : line) {
                if (ch < '0' || ch > '9') {
                    ch = '0';
                }
            }

            custom_level_layout.push_back(line);
            row_count++;
        }
        
        // if file has fewer than 11 rows add empty rows
        while (custom_level_layout.size() < BreakoutConfig::kRows) {
            custom_level_layout.push_back("0000000000");
        }
        
        file.close();
        return true;
    }

    // reset the ball to their starting position above the paddle
    // ball remains attached to the paddle until the player releases it
    // clears any existing balls (including multi-ball clones)
    void reset_ball(bool stick) {
        balls.clear();
        Ball ball;
        ball.radius = BreakoutConfig::kBallRadius;
        // position ball above the center of the paddle
        ball.pos = {paddle.pos.x + paddle.size.x * 0.5f, paddle.pos.y - ball.radius - BreakoutConfig::kBallStickOffset};
        ball.color = BreakoutConfig::kBallColor;
        ball_speed = BreakoutConfig::kBallSpeed;
        ball.vel = {0.0f, -ball_speed * ball_speed_multiplier};
        balls.push_back(ball);
        ball_stuck = stick;
    }

    // main game update loop called once per frame
    // updates ball physics, handles collisions (walls, paddle, blocks, powerups)
    // manages ball loss and level completion, normalizes ball speeds
    // return win/lose or none for normal continue
    UpdateResult update_game(float dt) {
        if (ball_stuck && !balls.empty()) {
            balls[0].pos = {paddle.pos.x + paddle.size.x * 0.5f, paddle.pos.y - balls[0].radius - 2.0f};
        } else {
            for (auto& ball : balls) {
                // update ball position
                ball.pos += ball.vel * dt;

                // wall collisions
                // left wall 
                if (ball.pos.x - ball.radius < 0.0f) {
                    ball.pos.x = ball.radius;
                    ball.vel.x = std::abs(ball.vel.x);
                }
                // right wall
                if (ball.pos.x + ball.radius > screen.x) {
                    ball.pos.x = screen.x - ball.radius;
                    ball.vel.x = -std::abs(ball.vel.x);
                }
                // top wall
                if (ball.pos.y - ball.radius < 0.0f) {
                    ball.pos.y = ball.radius;
                    ball.vel.y = std::abs(ball.vel.y);
                }

                // paddle collision 
                if (circle_intersects_rect(ball.pos, ball.radius, paddle)) {
                    float paddle_center = paddle.pos.x + paddle.size.x * 0.5f;
                    // distance from paddle center (-1 to 1) determines horizontal velocity
                    float distance = (ball.pos.x - paddle_center) / (paddle.size.x * 0.5f);
                    distance = glm::clamp(distance, -1.0f, 1.0f);
                    // move away from center -> more speed 
                    ball.vel.x = distance * ball_speed * ball_speed_multiplier;
                    // forces ball upward 
                    ball.vel.y = -std::abs(ball.vel.y);
                    // snaps ball right above the paddle to prevent sticking
                    ball.pos.y = paddle.pos.y - ball.radius - 1.0f;
                }

                // block collisions (one per ball per frame)
                for (auto& block : blocks) {
                    if (!block.alive) continue;
                    if (circle_intersects_rect(ball.pos, ball.radius, block.rect)) {
                        block.alive = false;
                        maybe_spawn_powerup(block.rect);
                        glm::vec2 rect_center = block.rect.pos + block.rect.size * 0.5f;
                        glm::vec2 diff = ball.pos - rect_center;
                        glm::vec2 half = block.rect.size * 0.5f;
                        float overlap_x = half.x - std::abs(diff.x); // hit left/right edge
                        float overlap_y = half.y - std::abs(diff.y); // hit top/bottom edge
                        if (piercing_timer <= 0.0f) {
                            if (overlap_x < overlap_y) {
                                // side hit: reverse horizontal velocity
                                ball.vel.x = -ball.vel.x;
                            } else {
                                // top/bottom hit: reverse vertical velocity
                                ball.vel.y = -ball.vel.y;
                            }
                        }
                        break;
                    }
                }
            }
        }

        // remove balls that fell below the screen
        std::vector<Ball> balls_still_in_play;
        balls_still_in_play.reserve(balls.size());
        for (const Ball& ball : balls) {
            if (ball.pos.y - ball.radius > screen.y) {
                // Ball is fully below the screen, so drop it.
            } else {
                balls_still_in_play.push_back(ball);
            }
        }
        balls = std::move(balls_still_in_play);

        // check lose condition: if no balls remain, lose a life and reset ball
        if (balls.empty()) {
            lives--;
            reset_ball(true);
            if (lives <= 0) {
                update_powerups(dt);
                update_powerup_effects(dt);
                return UpdateResult::GameOver;
            }
            update_powerups(dt);
            update_powerup_effects(dt);
            return UpdateResult::None;
        }

        // check win
        int remaining = 0;
        for (auto& block : blocks) {
            if (block.alive) remaining++;
        }
        if (remaining == 0) {
            update_visuals();
            normalize_ball_speeds(ball_speed * ball_speed_multiplier);
            update_powerups(dt);
            update_powerup_effects(dt);
            return UpdateResult::Win;
        }

        update_visuals();
        normalize_ball_speeds(ball_speed * ball_speed_multiplier);
        update_powerups(dt);
        update_powerup_effects(dt);
        return UpdateResult::None;
    }

    // spawns a powerup with a given probability when a block is destroyed
    // randomly selects the power-up type with weighted probabilities:
    //      PaddleWiden: 20%
    //      MultiBall: 20%
    //      SlowBall: 15%
    //      Piercing: 25%
    //      ExtraLife: 20%
    // powerup spawns at the blocks center and begins falling downward
    void maybe_spawn_powerup(const Rect& source) {
        // only spawn a powerup if the random roll is within % range. 
        if (dist01(rng) > BreakoutConfig::kPowerupSpawnChance) return; // 35% chance

        // assign powerup type based on rng
        PowerUp pu;
        float roll = dist01(rng);
        if (roll < BreakoutConfig::kPowerupWidenChance) {
            pu.type = PowerUp::Type::PaddleWiden; 
        } else if (roll < BreakoutConfig::kPowerupMultiballChance) {
            pu.type = PowerUp::Type::MultiBall; 
        } else if (roll < BreakoutConfig::kPowerupSlowChance) {
            pu.type = PowerUp::Type::SlowBall; 
        } else if (roll < BreakoutConfig::kPowerupPiercingChance) {
            pu.type = PowerUp::Type::Piercing; 
        } else {
            pu.type = PowerUp::Type::ExtraLife;
        }

        // spawn powerup at the center of the destroyed block
        pu.rect.size = BreakoutConfig::kPowerupSize;
        pu.rect.pos = {
            source.pos.x + source.size.x * 0.5f - pu.rect.size.x * 0.5f,
            source.pos.y + source.size.y * 0.5f - pu.rect.size.y * 0.5f
        };

        // set color based on powerup type
        if (pu.type == PowerUp::Type::PaddleWiden) {
            pu.rect.color = BreakoutConfig::kPowerupColorWiden; // blue
        } else if (pu.type == PowerUp::Type::MultiBall) {
            pu.rect.color = BreakoutConfig::kPowerupColorMultiball; // yellow
        } else if (pu.type == PowerUp::Type::SlowBall) {
            pu.rect.color = BreakoutConfig::kPowerupColorSlow; // orange
        } else if (pu.type == PowerUp::Type::Piercing) {
            pu.rect.color = BreakoutConfig::kPowerupColorPiercing; // red
        } else {
            pu.rect.color = BreakoutConfig::kPowerupColorExtraLife; // light red, fallback only 
        }
        
        pu.vel = {0.0f, BreakoutConfig::kPowerupSpeed};
        pu.active = true;
        powerups.push_back(pu);
    }

    // updates active powerups
    // move them downward and handle paddle collision
    // activate caught powerups
    void update_powerups(float dt) {
        powerup_caught_this_frame = false;  // reset flag each frame
        for (auto& pu : powerups) {
            if (!pu.active) continue;
            pu.rect.pos += pu.vel * dt; // move powerup downward
            if (pu.rect.pos.y > screen.y) {
                pu.active = false;
                continue;
            }
            // powerup is caught with the paddle 
            if (rect_intersects_rect(pu.rect, paddle)) {
                pu.active = false;
                powerup_caught_this_frame = true;
                // activate powerup based on type
                if (pu.type == PowerUp::Type::PaddleWiden) activate_paddle_widen();
                else if (pu.type == PowerUp::Type::MultiBall) activate_multiball();
                else if (pu.type == PowerUp::Type::SlowBall) activate_slow_ball();
                else if (pu.type == PowerUp::Type::Piercing) activate_piercing();
                else lives++; // updated in draw_lives()
            }
        }
    }


    // cleans up whichever powerup is active at the moment
    void clear_mutually_exclusive_powerups() {

        if (paddle_wide_timer > 0.0f) {
            paddle_wide_timer = 0.0f;
            paddle.size.x = paddle_base_width;
            paddle.size.y = BreakoutConfig::kPaddleSize.y;
            paddle.pos.x = glm::clamp(paddle.pos.x, 0.0f, screen.x - paddle.size.x);
        }
        if (slow_ball_timer > 0.0f) {
            slow_ball_timer = 0.0f;
            ball_speed_multiplier = 1.0f;
            normalize_ball_speeds(ball_speed);
        }
        if (piercing_timer > 0.0f) {
            piercing_timer = 0.0f;
        }
    }

    // active paddle widen powerup 
    // increases paddle width , sets a timer, clamps position to screen bounds
    void activate_paddle_widen() {
        clear_mutually_exclusive_powerups();
        paddle_wide_timer = BreakoutConfig::kPaddleWidenTimer;
        paddle.size.x = paddle_base_width * BreakoutConfig::kPaddleWidenScale;
        //paddle.size.y = BreakoutConfig::kPaddleSize.y * BreakoutConfig::kPaddleWidenScale;
        // dont change height, just width
        paddle.pos.x = glm::clamp(paddle.pos.x, 0.0f, screen.x - paddle.size.x);
    }


    // activate multi-ball powerup
    // spawns 2 additional balls with the same speed as the original but different angles
    void activate_multiball() {
        if (balls.empty()) return;
        glm::vec2 base_dir = balls[0].vel;
        // normalize to unit direction
        float len = std::sqrt(base_dir.x * base_dir.x + base_dir.y * base_dir.y);
        if (len < 0.001f) {
            base_dir = {0.0f, -1.0f};
        } else {
            base_dir /= len;
        }

        // compute rotation for new ball directions
        const float angle = BreakoutConfig::kMultiBallAngle; // ~20 degrees
        float c = std::cos(angle);
        float s = std::sin(angle);

        glm::vec2 dir1 = {base_dir.x * c - base_dir.y * s, base_dir.x * s + base_dir.y * c};
        glm::vec2 dir2 = {base_dir.x * c + base_dir.y * s, -base_dir.x * s + base_dir.y * c};
        
        // clone ball twice and assign new velocities
        Ball b1 = balls[0];
        Ball b2 = balls[0];
        b1.vel = dir1 * ball_speed * ball_speed_multiplier;
        b2.vel = dir2 * ball_speed * ball_speed_multiplier;
        balls.push_back(b1);
        balls.push_back(b2);
    }

    // activate the slow-ball powerup
    // reduces the ball speed multiplier, sets a timer
    void activate_slow_ball() {
        clear_mutually_exclusive_powerups();
        slow_ball_timer = BreakoutConfig::kSlowBallTimer;
        ball_speed_multiplier = BreakoutConfig::kSlowBallMultiplier;
        normalize_ball_speeds(ball_speed * ball_speed_multiplier);
    }

    // activate the piercing powerup
    // enables balls to pass through blocks without bouncing
    void activate_piercing() {
        clear_mutually_exclusive_powerups();
        piercing_timer = BreakoutConfig::kPiercingTimer;
    }

    // restore game state when powerups expires (paddle size, ball speed, piercing off)
    void update_powerup_effects(float dt) {

        // counddown and cancellation of powerup, when expired
        
        if (paddle_wide_timer > 0.0f) {
            paddle_wide_timer -= dt; // count timer down
            // shrink paddle back to normal size
            if (paddle_wide_timer <= 0.0f) {
                paddle_wide_timer = 0.0f;
                paddle.size.x = paddle_base_width;
                paddle.size.y = BreakoutConfig::kPaddleSize.y; 
                paddle.pos.x = glm::clamp(paddle.pos.x, 0.0f, screen.x - paddle.size.x);
            }
        }
        if (slow_ball_timer > 0.0f) {
            slow_ball_timer -= dt; 
            // reset ball speed to normal
            if (slow_ball_timer <= 0.0f) {
                slow_ball_timer = 0.0f;
                ball_speed_multiplier = 1.0f;
                normalize_ball_speeds(ball_speed);
            }
        }
        if (piercing_timer > 0.0f) {
            piercing_timer -= dt;
            // disable piercing effect
            if (piercing_timer < 0.0f) piercing_timer = 0.0f;
        }
    }

    // Update visual appearance based on active powerup effects
    // changes paddle color based on active powerup, ball appearance stays normal even during slow ball
    void update_visuals() {
        // paddle color: priority is piercing (red) > slow (orange) > widen (blue) > normal (grey)
        if (piercing_timer > 0.0f) {
            paddle.color = BreakoutConfig::kPowerupColorPiercing;
        } else if (slow_ball_timer > 0.0f) {
            paddle.color = BreakoutConfig::kPowerupColorSlow;
        } else if (paddle_wide_timer > 0.0f) {
            paddle.color = BreakoutConfig::kPowerupColorWiden;
        } else {
            paddle.color = BreakoutConfig::kPaddleColor;
        }

        // ball color: always normal grey regardless of powerups
        glm::vec4 ball_color = BreakoutConfig::kBallColor;
        for (auto& ball : balls) {
            ball.color = ball_color;
        }
    }

    // recalculate ball speeds
    // keeps balls moving at consistent speed without changing direction
    void normalize_ball_speeds(float target_speed) {
        for (auto& ball : balls) {
            float len = std::sqrt(ball.vel.x * ball.vel.x + ball.vel.y * ball.vel.y); // velocity magnitude 
            if (len > 0.001f) {
                ball.vel = (ball.vel / len) * target_speed;
            }
        }
    }

    // test collision between a circle(ball) and a rectangle. 
    bool circle_intersects_rect(const glm::vec2& center, float radius, const Rect& rect) {
        
        // rectangle center
        glm::vec2 rect_center = rect.pos + rect.size * 0.5f;
        glm::vec2 half = rect.size * 0.5f;
  
        glm::vec2 diff = center - rect_center; // relative position of circle

        glm::vec2 clamped = glm::clamp(diff, -half, half); // rectangle bounds relative to center
        glm::vec2 closest = rect_center + clamped; // closest point on rectangle
       
        // distance from closest point to circle center
        glm::vec2 delta = center - closest; 
        return glm::dot(delta, delta) <= radius * radius;
    }

    // test collision between two rectangles (AABB).
    // on both the X and Y axes simultaneously.
    bool rect_intersects_rect(const Rect& a, const Rect& b) {
        return a.pos.x < b.pos.x + b.size.x && // a's left edge is left of b's right edge
               a.pos.x + a.size.x > b.pos.x && // a's right edge is right of b's left edge
               a.pos.y < b.pos.y + b.size.y && // a's top edge is above b's bottom edge
               a.pos.y + a.size.y > b.pos.y; // a's bottom edge is below b's top edge
    }
};
