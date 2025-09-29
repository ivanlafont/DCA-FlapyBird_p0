// src/MainGameState.cpp
extern "C" {
    #include <raylib.h>
}
#include "MainGameState.hpp"
#include "StateMachine.hpp"
#include "GameOverState.hpp"
#include <memory>
#include <string>

// helper para destino
static Rectangle make_dest(float x, float y, float w, float h) {
    return Rectangle{ x, y, w, h };
}

MainGameState::MainGameState(StateMachine* sm) : sm_(sm) {
    bird_.x = 200.0f;
    bird_.y = 200.0f;
    SetRandomSeed(GetTime());

    // --- fondos ---
    texBg_[0] = LoadTexture("assets/background-day.png");
    texBg_[1] = LoadTexture("assets/background-night.png");

    // --- suelo ---
    texGround_ = LoadTexture("assets/base.png");

    // --- pájaros (red/blue/yellow × down/mid/up) ---
    const char* colors[3] = {"red","blue","yellow"};
    const char* flaps[3]  = {"downflap","midflap","upflap"};
    for (int c=0; c<3; c++) {
        for (int f=0; f<3; f++) {
            std::string path = "assets/" + std::string(colors[c]) + "bird-" + flaps[f] + ".png";
            texBirds_[c][f] = LoadTexture(path.c_str());
        }
    }
    birdColor_ = GetRandomValue(0,2);

    // --- tuberías ---
    texPipes_[0] = LoadTexture("assets/pipe-green.png");
    texPipes_[1] = LoadTexture("assets/pipe-red.png");
    pipeColor_ = GetRandomValue(0,1);

    // --- dígitos ---
    for (int i=0; i<10; i++) {
        std::string path = "assets/" + std::to_string(i) + ".png";
        texDigits_[i] = LoadTexture(path.c_str());
    }

    // --- pantalla “get ready” (opcional) ---
    texMessage_ = LoadTexture("assets/message.png");
}

MainGameState::~MainGameState() {
    for (auto& t : texBg_) if (t.id) UnloadTexture(t);
    if (texGround_.id) UnloadTexture(texGround_);
    for (int c=0; c<3; c++) for (int f=0; f<3; f++) if (texBirds_[c][f].id) UnloadTexture(texBirds_[c][f]);
    for (auto& t : texPipes_) if (t.id) UnloadTexture(t);
    for (auto& t : texDigits_) if (t.id) UnloadTexture(t);
    if (texMessage_.id) UnloadTexture(texMessage_);
}

void MainGameState::handleInput() {
    if (IsKeyPressed(KEY_SPACE)) bird_.vy += JUMP_;
    if (IsKeyPressed(KEY_F1)) debugBoxes_ = !debugBoxes_;
}

void MainGameState::update(float dt) {
    // --- física ---
    bird_.vy += GRAVITY_ * dt;
    bird_.y  += bird_.vy * dt;
    bird_.vy  = 0.0f;

    // límites pantalla -> game over
    if (bird_.y - bird_.r < 0 || bird_.y + bird_.r > GetScreenHeight()) {
        sm_->add_state(std::make_unique<GameOverState>(sm_, score_), true);
        return;
    }

    // --- animación bird ---
    birdAnimTimer_ += dt;
    if (birdAnimTimer_ >= 0.15f) {
        birdAnimTimer_ = 0.0f;
        birdFrame_ = (birdFrame_ + 1) % 3;
    }

    // --- scroll fondo/suelo ---
    bgX_     -= BG_SPEED_ * dt;
    groundX_ -= GROUND_SPEED_ * dt;
    if (bgX_ <= -texBg_[0].width) bgX_ = 0.0f;
    if (groundX_ <= -texGround_.width) groundX_ = 0.0f;

    // --- spawner pipes ---
    spawnTimer_ += dt;
    if (spawnTimer_ >= spawnEvery_) {
        spawnTimer_ = 0.0f;
        int minOff = PIPE_H_/2;
        int maxOff = GetScreenHeight()/2;
        int pipe_y_offset_top = GetRandomValue(minOff, maxOff);
        float centerY = (PIPE_H_ - pipe_y_offset_top)
                      + GetRandomValue(PIPE_H_/2, GetScreenHeight()/2);
        spawnPipe(centerY);
    }

    // mover pipes
    for (auto& p : pipes_) {
        p.top.x -= PIPE_SPEED_ * dt;
        p.bot.x -= PIPE_SPEED_ * dt;
    }
    while (!pipes_.empty() && (pipes_.front().top.x + PIPE_W_ < 0)) {
        pipes_.pop_front();
    }

    // colisiones + score
    Rectangle birdBB{ bird_.x - bird_.r, bird_.y - bird_.r, bird_.r*2, bird_.r*2 };
    for (auto& p : pipes_) {
        if (CheckCollisionRecs(birdBB, p.top) || CheckCollisionRecs(birdBB, p.bot)) {
            sm_->add_state(std::make_unique<GameOverState>(sm_, score_), true);
            return;
        }
        if (!p.scored && (p.top.x + PIPE_W_ < bird_.x)) {
            p.scored = true;
            score_++;
        }
    }
}

void MainGameState::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // fondo (usa day/night según birdColor_ para variar)
    DrawTexture(texBg_[birdColor_%2], (int)bgX_, 0, WHITE);
    DrawTexture(texBg_[birdColor_%2], (int)bgX_ + texBg_[0].width, 0, WHITE);

    // tuberías
    Texture2D currentPipe = texPipes_[pipeColor_];
    Rectangle srcPipe = {0, 0, (float)currentPipe.width, (float)currentPipe.height};
    for (const auto& p : pipes_) {
        Rectangle srcTop = {0, (float)currentPipe.height, (float)currentPipe.width, -(float)currentPipe.height};
        Rectangle dstTop = make_dest(p.top.x, p.top.y, (float)PIPE_W_, (float)PIPE_H_);
        DrawTexturePro(currentPipe, srcTop, dstTop, Vector2{0,0}, 0.0f, WHITE);

        Rectangle dstBot = make_dest(p.bot.x, p.bot.y, (float)PIPE_W_, (float)PIPE_H_);
        DrawTexturePro(currentPipe, srcPipe, dstBot, Vector2{0,0}, 0.0f, WHITE);
    }

    // pájaro animado
    Texture2D currentBird = texBirds_[birdColor_][birdFrame_];
    Rectangle srcBird = {0, 0, (float)currentBird.width, (float)currentBird.height};
    float birdW = bird_.r * 2.0f;
    float birdH = bird_.r * 2.0f;
    Rectangle dstBird = {bird_.x - birdW/2.0f, bird_.y - birdH/2.0f, birdW, birdH};
    DrawTexturePro(currentBird, srcBird, dstBird, Vector2{0,0}, 0.0f, WHITE);

    // suelo
    int groundY = GetScreenHeight() - texGround_.height;
    DrawTexture(texGround_, (int)groundX_, groundY, WHITE);
    DrawTexture(texGround_, (int)groundX_ + texGround_.width, groundY, WHITE);

    // puntuación con dígitos
    std::string s = std::to_string(score_);
    int x = GetScreenWidth()/2 - (s.size()*texDigits_[0].width)/2;
    for (char ch : s) {
        int d = ch - '0';
        DrawTexture(texDigits_[d], x, 20, WHITE);
        x += texDigits_[d].width;
    }

    EndDrawing();
}

void MainGameState::spawnPipe(float gapCenterY) {
    float x = (float)GetScreenWidth();
    Rectangle top{ x, - (float)GetRandomValue(PIPE_H_/2, GetScreenHeight()/2), (float)PIPE_W_, (float)PIPE_H_ };
    Rectangle bot{ x, gapCenterY + GAP_/2.0f, (float)PIPE_W_, (float)PIPE_H_ };
    pipes_.push_back(PipePair{ top, bot, false });
}

