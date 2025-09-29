extern "C" {
    #include <raylib.h>
}

#include "StateMachine.hpp"
#include "MainGameState.hpp"
#include <memory>

int main() {
    const int W = 288;
    const int H = 512;

    InitWindow(W, H, "Flappy Bird DCA");
    SetTargetFPS(60);

    StateMachine sm;
    sm.add_state(std::make_unique<MainGameState>(&sm), false);

    while (!sm.is_game_ending() && !WindowShouldClose()) {
        float dt = GetFrameTime();

        sm.handle_state_changes(dt);

        auto* st = sm.getCurrentState().get();
        if (st) {
            st->handleInput();
            st->update(dt);
            st->render();
        }
    }

    CloseWindow();
    return 0;
}

