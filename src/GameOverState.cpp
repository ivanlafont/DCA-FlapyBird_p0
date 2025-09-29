extern "C" {
    #include <raylib.h>
}
#include "GameOverState.hpp"
#include "MainGameState.hpp"
#include "StateMachine.hpp"
#include <memory>
#include <string>

GameOverState::GameOverState(StateMachine* sm, int finalScore)
: sm_(sm), score_(finalScore) {
    texGameOver_ = LoadTexture("assets/gameover.png");
    SetTextureFilter(texGameOver_, TEXTURE_FILTER_BILINEAR);
}

GameOverState::~GameOverState() {
    if (texGameOver_.id) UnloadTexture(texGameOver_);
}

void GameOverState::handleInput() {
    if (IsKeyPressed(KEY_SPACE)) {
        sm_->add_state(std::make_unique<MainGameState>(sm_), true);
    }
}

void GameOverState::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    int x = GetScreenWidth()/2 - texGameOver_.width/2;
    int y = GetScreenHeight()/2 - texGameOver_.height/2;
    DrawTexture(texGameOver_, x, y, WHITE);

    std::string s = "Score: " + std::to_string(score_);
    int sw = MeasureText(s.c_str(), 24);
    DrawText(s.c_str(), (GetScreenWidth()-sw)/2, GetScreenHeight()-50, 24, BLACK);

    EndDrawing();
}

