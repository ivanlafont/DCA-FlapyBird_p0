#pragma once
#include <deque>
#include <memory>
#include <string>

#include "GameState.hpp"
class StateMachine;

extern "C" {
    #include <raylib.h>
}

struct Bird {
    float x{200.0f};
    float y{200.0f};
    float vy{0.0f};
    float r{17.0f};
};

struct PipePair {
    Rectangle top;
    Rectangle bot;
    bool scored{false};
};

class MainGameState : public GameState {
public:
    explicit MainGameState(StateMachine* sm);
    ~MainGameState();

    void init() override {}
    void pause() override {}
    void resume() override {}

    void handleInput() override;
    void update(float dt) override;
    void render() override;

private:
    void spawnPipe(float gapCenterY);

    StateMachine* sm_{nullptr};

    Bird bird_;
    std::deque<PipePair> pipes_;
    float spawnTimer_{0.0f};
    float spawnEvery_{1.5f};

    const float GRAVITY_ = 2900.0f;
    const float JUMP_    = -1900.0f;
    const float PIPE_SPEED_ = 120.0f;
    const int   PIPE_W_ = 52;
    const int   PIPE_H_ = 320;
    const int   GAP_    = 100;

    int score_{0};
    bool debugBoxes_{false};

    // --- Sprites ---
    Texture2D texBg_[2];           // day/night
    Texture2D texGround_;
    Texture2D texBirds_[3][3];     // 3 colores × 3 frames
    Texture2D texPipes_[2];        // verde/rojo
    Texture2D texDigits_[10];
    Texture2D texMessage_;
    Texture2D texGameOver_;

    int birdColor_{0};
    int birdFrame_{0};
    float birdAnimTimer_{0.0f};

    int pipeColor_{0};

    float bgX_{0.0f};
    float groundX_{0.0f};
    const float BG_SPEED_ = 20.0f;
    const float GROUND_SPEED_ = 120.0f;
};

