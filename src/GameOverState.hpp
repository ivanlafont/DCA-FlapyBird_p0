#pragma once
#include "GameState.hpp"
class StateMachine;
extern "C" {
    #include <raylib.h>
}

class GameOverState : public GameState {
public:
    GameOverState(StateMachine* sm, int finalScore);
    ~GameOverState(); 

    void init() override {}
    void pause() override {}
    void resume() override {}

    void handleInput() override;
    void update(float) override {}
    void render() override;

private:
    StateMachine* sm_{nullptr};
    int score_{0};
    Texture2D texGameOver_{};
};

