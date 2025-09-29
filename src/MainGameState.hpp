#pragma once
#include <deque>
#include <memory>
#include <string>
#include <algorithm>

#include "GameState.hpp"
class StateMachine;

extern "C" {
    #include <raylib.h>
}

// x,y = esquina superior-izquierda del sprite del pájaro
struct Bird {
    float x{120.0f};
    float y{200.0f};
    float vy{0.0f};
    int   width{0};
    int   height{0};
};

struct PipePair {
    Rectangle top;
    Rectangle bot;
    bool scored{false};
    bool red{false};     // color de esta pareja (false=green, true=red)
};

class MainGameState : public GameState {
public:
    explicit MainGameState(StateMachine* sm);
    ~MainGameState();

    void init() override;   // aquí cargamos sprites y fijamos tamaños
    void pause() override {}
    void resume() override {}

    void handleInput() override;
    void update(float dt) override;
    void render() override;

private:
    void spawnPipe();  // generará centro y color válidos

    StateMachine* sm_{nullptr};

    // --- Jugador
    Bird bird_;

    // Lo que pide la práctica: sprites "actuales"
    Texture2D birdSprite{};   // frame actual del pájaro
    Texture2D pipeSprite{};   // (no se usa para dibujar todas; mantenemos por requisito)

    // Aleteo y paletas (se usan para actualizar birdSprite)
    Texture2D birdFrames_[3][3]{}; // [color][frame] => 0:red,1:blue,2:yellow × 0:down,1:mid,2:up
    int  birdColor_{0};
    int  birdFrame_{0};
    float birdAnimTimer_{0.0f};

    // Fondos, suelo y dígitos (para usar todos los PNG)
    Texture2D texBg_[2]{};     // 0:day, 1:night
    int bgIdx_{0};
    Texture2D texGround_{};
    Texture2D texDigits_[10]{};

    // Tuberías (dos colores)
    Texture2D pipeGreen_{};
    Texture2D pipeRed_{};

    // Cola de tuberías
    std::deque<PipePair> pipes_;

    // Spawner
    float spawnTimer_{0.0f};
    float spawnEvery_{1.25f}; // s (un poco más ágil)

    // Física básica
    const float GRAVITY_ = 6500.0f;
    const float JUMP_    = -2600.0f;

    // Parámetros de tuberías (ya NO const, la práctica lo pide variable)
    float PIPE_SPEED_ = 140.0f; // px/s
    float PIPE_W = 0.0f;        // se fija en init() con el sprite
    float PIPE_H = 0.0f;        // se fija en init() con el sprite
    float GAP    = 0.0f;        // se fija en init(): max(bird.h*4.5, 96)

    // Márgenes para que nunca salgan imposibles
    const float GAP_MULT_   = 5.0f;   // más generoso que 4.5
    const float GAP_MIN_PX_ = 96.0f;
    const float GAP_MARGIN_ = 20.0f;

    // Scroll estético
    float bgX_{0.0f};
    float groundX_{0.0f};
    const float BG_SPEED_ = 20.0f;
    const float GROUND_SPEED_ = 140.0f;

    // Puntuación y debug
    int  score_{0};
    bool debugBoxes_{false};
};

