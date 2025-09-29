extern "C" {
    #include <raylib.h>
}
#include "MainGameState.hpp"
#include "StateMachine.hpp"
#include "GameOverState.hpp"
#include <memory>
#include <string>

MainGameState::MainGameState(StateMachine* sm) : sm_(sm) {}

MainGameState::~MainGameState() {
    // Libera TODO lo cargado
    for (auto& t : texBg_) if (t.id) UnloadTexture(t);
    if (texGround_.id) UnloadTexture(texGround_);
    for (int c=0;c<3;c++) for (int f=0;f<3;f++) if (birdFrames_[c][f].id) UnloadTexture(birdFrames_[c][f]);
    for (auto& t : texDigits_) if (t.id) UnloadTexture(t);
    if (pipeGreen_.id) UnloadTexture(pipeGreen_);
    if (pipeRed_.id)   UnloadTexture(pipeRed_);
    if (birdSprite.id) UnloadTexture(birdSprite);   // por si quedó copiado
    if (pipeSprite.id) UnloadTexture(pipeSprite);   // idem
}

void MainGameState::init() {
    SetRandomSeed(GetTime());

    // --- Fondos y suelo (usar day/night y base.png)
    texBg_[0]    = LoadTexture("assets/background-day.png");
    texBg_[1]    = LoadTexture("assets/background-night.png");
    bgIdx_       = GetRandomValue(0,1);
    texGround_   = LoadTexture("assets/base.png");

    // --- Dígitos 0..9
    for (int i=0;i<10;i++) {
        texDigits_[i] = LoadTexture(std::string("assets/" + std::to_string(i) + ".png").c_str());
    }

    // --- Pájaros: 3 colores × 3 frames
    const char* colors[3] = {"red","blue","yellow"};
    const char* flaps[3]  = {"downflap","midflap","upflap"};
    for (int c=0;c<3;c++) {
        for (int f=0;f<3;f++) {
            std::string path = std::string("assets/") + colors[c] + "bird-" + flaps[f] + ".png";
            birdFrames_[c][f] = LoadTexture(path.c_str());
        }
    }
    birdColor_ = GetRandomValue(0,2);
    birdFrame_ = 1; // mid
    birdAnimTimer_ = 0.0f;

    // La práctica pide birdSprite: usamos el frame actual para cumplir requisito
    birdSprite = birdFrames_[birdColor_][birdFrame_];

    // Fijar tamaño del jugador por sprite
    bird_.width  = birdSprite.width;
    bird_.height = birdSprite.height;

    // Posición inicial un poco hacia la derecha y centrada vertical
    bird_.x = 128.0f;
    bird_.y = GetScreenHeight()*0.42f;

    // --- Tuberías: dos colores y parámetros dimensiones
    pipeGreen_ = LoadTexture("assets/pipe-green.png");
    pipeRed_   = LoadTexture("assets/pipe-red.png");

    // La práctica pide pipeSprite (variable del estado). Le damos uno por defecto.
    pipeSprite = pipeGreen_;

    PIPE_W = (float)pipeGreen_.width;   // ambos pipes suelen tener mismo tamaño
    PIPE_H = (float)pipeGreen_.height;

    GAP = bird_.height * 4.5f;
}

void MainGameState::handleInput() {
    if (IsKeyPressed(KEY_SPACE)) bird_.vy += JUMP_;
    if (IsKeyPressed(KEY_F1))    debugBoxes_ = !debugBoxes_;
}

void MainGameState::update(float dt) {
    // Física del pájaro (según enunciado)
    bird_.vy += GRAVITY_ * dt;
    bird_.y  += bird_.vy * dt;
    bird_.vy  = 0.0f;

    // Aleteo: actualiza frame y birdSprite (cumplimos requisito de propiedad)
    birdAnimTimer_ += dt;
    if (birdAnimTimer_ >= 0.15f) {
        birdAnimTimer_ = 0.0f;
        birdFrame_ = (birdFrame_ + 1) % 3;
        birdSprite = birdFrames_[birdColor_][birdFrame_];
        // (tamaño se mantiene; si cambiaran, podrías re-actualizar width/height aquí)
    }

    // Salida de pantalla → Game Over
    if (bird_.y < 0 || bird_.y + bird_.height > GetScreenHeight() - (texGround_.id ? texGround_.height : 0)) {
        sm_->add_state(std::make_unique<GameOverState>(sm_, score_), true);
        return;
    }

    // Scroll estético
    bgX_     -= BG_SPEED_     * dt;
    groundX_ -= GROUND_SPEED_ * dt;
    if (bgX_     <= -texBg_[bgIdx_].width)  bgX_ = 0.0f;
    if (groundX_ <= -texGround_.width)      groundX_ = 0.0f;

    // Spawner
    spawnTimer_ += dt;
    if (spawnTimer_ >= spawnEvery_) {
        spawnTimer_ = 0.0f;
        spawnPipe();
    }

    // Mover tuberías
    for (auto &p : pipes_) {
        p.top.x -= PIPE_SPEED_ * dt;
        p.bot.x -= PIPE_SPEED_ * dt;
    }

    // Borrar las que salieron
    while (!pipes_.empty() && (pipes_.front().top.x + PIPE_W < 0)) {
        pipes_.pop_front();
    }

    // AABB del jugador (ahora por width/height)
    Rectangle playerBB{ bird_.x, bird_.y, (float)bird_.width, (float)bird_.height };

    // Colisiones + puntuación
    for (auto &p : pipes_) {
        if (CheckCollisionRecs(playerBB, p.top) || CheckCollisionRecs(playerBB, p.bot)) {
            sm_->add_state(std::make_unique<GameOverState>(sm_, score_), true);
            return;
        }
        if (!p.scored && (p.top.x + PIPE_W < bird_.x)) {
            p.scored = true;
            score_++;
        }
    }
}

void MainGameState::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Fondo (tileado)
    DrawTexture(texBg_[bgIdx_], (int)bgX_, 0, WHITE);
    DrawTexture(texBg_[bgIdx_], (int)bgX_ + texBg_[bgIdx_].width, 0, WHITE);

    // Pájaro (lo que pide la práctica: DrawTexture con birdSprite)
    DrawTexture(birdSprite, (int)bird_.x, (int)bird_.y, WHITE);

    // Tuberías (lo que pide la práctica: DrawTextureEx con 180º en la de arriba)
    for (const auto& p : pipes_) {
        const Texture2D& t = p.red ? pipeRed_ : pipeGreen_;

        // Superior → rotada 180º, con offset (x+PIPE_W, y+PIPE_H)
        Vector2 posTop{ p.top.x + PIPE_W, p.top.y + PIPE_H };
        DrawTextureEx(t, posTop, 180.0f, 1.0f, WHITE);

        // Inferior → normal en (x, y)
        Vector2 posBot{ p.bot.x, p.bot.y };
        DrawTextureEx(t, posBot, 0.0f, 1.0f, WHITE);
    }

    // Suelo (tileado al fondo)
    int groundY = GetScreenHeight() - texGround_.height;
    DrawTexture(texGround_, (int)groundX_, groundY, WHITE);
    DrawTexture(texGround_, (int)groundX_ + texGround_.width, groundY, WHITE);

    // Puntuación con sprites 0..9 (centrada arriba)
    std::string s = std::to_string(score_);
    int totalW = 0;
    for (char ch : s) totalW += texDigits_[ch-'0'].width;
    int x = GetScreenWidth()/2 - totalW/2;
    for (char ch : s) {
        int d = ch - '0';
        DrawTexture(texDigits_[d], x, 12, WHITE);
        x += texDigits_[d].width;
    }

    // Debug
    if (debugBoxes_) {
        DrawRectangleLinesEx(Rectangle{bird_.x,bird_.y,(float)bird_.width,(float)bird_.height}, 2, BLUE);
        for (const auto& p : pipes_) {
            DrawRectangleLinesEx(p.top, 2, RED);
            DrawRectangleLinesEx(p.bot, 2, RED);
        }
    }

    EndDrawing();
}

void MainGameState::spawnPipe() {
    const float screenH = (float)GetScreenHeight();
    const float groundH = texGround_.id ? (float)texGround_.height : 0.0f;

    // Centro del hueco restringido por márgenes y suelo
    const float minCenter = GAP_MARGIN_ + GAP * 0.5f;
    const float maxCenter = screenH - groundH - GAP_MARGIN_ - GAP * 0.5f;
    float gapCenterY = (float)GetRandomValue((int)minCenter, (int)maxCenter);

    float x = (float)GetScreenWidth();
    float topY = gapCenterY - GAP * 0.5f - PIPE_H;
    float botY = gapCenterY + GAP * 0.5f;

    PipePair pp;
    pp.top = Rectangle{ x, topY, PIPE_W, PIPE_H };
    pp.bot = Rectangle{ x, botY, PIPE_W, PIPE_H };
    pp.red = GetRandomValue(0,1) == 1; // usa pipe rojo o verde

    pipes_.push_back(pp);
}

