#include "PrototypeSurvivalScene.h"
#include "SceneManager.h" // 必要に応じてinclude
#include "WindowSize.h"
#include "Novice.h"
#include <cmath>
#include <algorithm>

const float PI = 3.14159265f;

// 色定義
const unsigned int COLOR_PLAYER = 0xAAAAFFFF;
const unsigned int COLOR_ENEMY_NORMAL = 0xFF4444FF;
const unsigned int COLOR_ENEMY_TANK = 0x880000FF;
const unsigned int COLOR_DEBRIS_SAFE = 0x00FF00FF;
const unsigned int COLOR_DEBRIS_EXPAND = 0xFFFF00FF;
const unsigned int COLOR_DEBRIS_ATTACK = 0xFF0000FF;
const unsigned int COLOR_DEBRIS_COOLDOWN = 0x555555FF;

// コンストラクタ修正：Managerを受け取る
PrototypeSurvivalScene::PrototypeSurvivalScene(SceneManager& manager)
    : manager_(&manager) {

    input_ = std::make_unique<InputManager>();

    player_.pos = { kWindowWidth / 2.0f, kWindowHeight / 2.0f };

    // デブリ初期化
    debris_.currentRadius = debris_.minRadius;
    debris_.state = DebrisState::Idle;
    debris_.pieces.resize(debris_.debrisCount);

    float angleStep = 2.0f * PI / debris_.debrisCount;
    for (int i = 0; i < debris_.debrisCount; i++) {
        debris_.pieces[i].angleOffset = i * angleStep;

        // バラつき生成
        debris_.pieces[i].distNoise = (float)(rand() % 40 - 20);
        debris_.pieces[i].speedScale = 0.8f + (float)(rand() % 60) / 100.0f;
        debris_.pieces[i].selfRotSpeed = (float)(rand() % 100 - 50) / 10.0f;
        debris_.pieces[i].currentSelfAngle = 0.0f;
    }

    enemies_.resize(100);
}

PrototypeSurvivalScene::~PrototypeSurvivalScene() {}

void PrototypeSurvivalScene::Update(float deltaTime, const char* keys, const char* preKeys) {
	keys; preKeys; // 未使用警告回避
    // InputManagerの更新
    // ※今回はInputManagerを独自に持たせているが、
    // 引数のkeys/preKeysを使いたい場合はここを書き換えても良い
    input_->Update();

    if (hitStopTimer_ > 0.0f) {
        hitStopTimer_ -= deltaTime;
        if (hitStopTimer_ <= 0.0f) hitStopTimer_ = 0.0f;
        else return;
    }

    if (screenShakeTimer_ > 0.0f) {
        screenShakeTimer_ -= deltaTime;
        cameraOffset_.x = (float)(rand() % 100 - 50) / 50.0f * screenShakePower_;
        cameraOffset_.y = (float)(rand() % 100 - 50) / 50.0f * screenShakePower_;
    }
    else {
        cameraOffset_ = { 0,0 };
    }

    UpdatePlayer(deltaTime);
    UpdateDebris(deltaTime);
    UpdateEnemies(deltaTime);
    CheckCollisions();
}

void PrototypeSurvivalScene::UpdatePlayer(float dt) {
    Vector2 moveDir = { 0,0 };
    if (input_->PressKey(DIK_W)) moveDir.y -= 1.0f;
    if (input_->PressKey(DIK_S)) moveDir.y += 1.0f;
    if (input_->PressKey(DIK_A)) moveDir.x -= 1.0f;
    if (input_->PressKey(DIK_D)) moveDir.x += 1.0f;

    if (input_->GetInputMode() == InputMode::Gamepad) {
        moveDir = input_->GetPad()->GetLeftStick();
    }

    if (Vector2::Length(moveDir) > 0.0f) {
        moveDir = Vector2::Normalize(moveDir);
        player_.pos += moveDir * 300.0f * dt;
    }

    player_.pos.x = std::clamp(player_.pos.x, player_.radius, kWindowWidth - player_.radius);
    player_.pos.y = std::clamp(player_.pos.y, player_.radius, kWindowHeight - player_.radius);

    if (player_.invincibilityTimer > 0.0f) {
        player_.invincibilityTimer -= dt;
    }
}

void PrototypeSurvivalScene::UpdateDebris(float dt) {
    debris_.rotationAngle += debris_.rotationSpeed * dt;

    bool isSpacePressed = input_->PressKey(DIK_SPACE) || input_->GetPad()->Press(Pad::Button::A);

    for (auto& p : debris_.pieces) {
        p.currentSelfAngle += p.selfRotSpeed * dt;
    }

    switch (debris_.state) {
    case DebrisState::Idle:
        debris_.currentRadius = debris_.minRadius;
        if (isSpacePressed) {
            debris_.state = DebrisState::Expanding;
            debris_.isCritical = false;
        }
        break;

    case DebrisState::Expanding:
        debris_.currentRadius += debris_.expandSpeed * dt;
        if (debris_.currentRadius >= debris_.maxRadius) {
            debris_.currentRadius = debris_.maxRadius;
            debris_.state = DebrisState::WaitMax;
            debris_.isCritical = true;
        }
        if (!isSpacePressed) debris_.state = DebrisState::Contracting;
        break;

    case DebrisState::WaitMax:
        debris_.currentRadius = debris_.maxRadius;
        if (!isSpacePressed) debris_.state = DebrisState::Contracting;
        break;

    case DebrisState::Contracting:
    {
        float speed = debris_.contractSpeed;
        if (debris_.isCritical) speed *= 1.5f;
        debris_.currentRadius -= speed * dt;
        if (debris_.currentRadius <= debris_.minRadius) {
            debris_.currentRadius = debris_.minRadius;
            debris_.state = DebrisState::Cooldown;
            debris_.cooldownTimer = 0.5f;
            StartShake(0.1f, 5.0f);
        }
    }
    break;

    case DebrisState::Cooldown:
        debris_.cooldownTimer -= dt;
        if (debris_.cooldownTimer <= 0.0f) debris_.state = DebrisState::Idle;
        break;
    }
}

void PrototypeSurvivalScene::UpdateEnemies(float dt) {
    enemySpawnTimer_ += dt;
    float spawnRate = 0.5f;
    if (enemySpawnTimer_ > spawnRate) {
        enemySpawnTimer_ = 0.0f;
        SpawnEnemy();
    }

    for (auto& e : enemies_) {
        if (!e.isAlive) continue;

        if (e.knockbackDuration > 0.0f) {
            e.knockbackDuration -= dt;
            e.pos += e.knockbackVel * dt;
            e.knockbackVel *= 0.9f;
        }
        else {
            Vector2 toPlayer = player_.pos - e.pos;
            if (Vector2::Length(toPlayer) > 0.1f) {
                Vector2 dir = Vector2::Normalize(toPlayer);
                float speed = (e.type == EnemyType::Normal) ? 100.0f : 50.0f;
                e.velocity = dir * speed;
                e.pos += e.velocity * dt;
            }
        }
    }
}

float PrototypeSurvivalScene::NormalizeAngle(float angle) {
    while (angle > PI) angle -= 2.0f * PI;
    while (angle < -PI) angle += 2.0f * PI;
    return angle;
}

void PrototypeSurvivalScene::CheckCollisions() {
    float debrisCollisionRadius = debris_.debrisSize * 0.6f;

    for (auto& e : enemies_) {
        if (!e.isAlive) continue;

        // 1. プレイヤーとの衝突
        float distToCore = Vector2::Length(player_.pos - e.pos);
        if (distToCore < player_.radius + e.radius) {
            if (player_.invincibilityTimer <= 0.0f) {
                player_.hp--;
                player_.invincibilityTimer = 1.0f;
                StartShake(0.3f, 15.0f);
                e.knockbackVel = Vector2::Normalize(e.pos - player_.pos) * 500.0f;
                e.knockbackDuration = 0.2f;
            }
        }

        // 2. デブリとの衝突（精密判定・隙間あり）
        Vector2 toEnemy = e.pos - player_.pos;
        float enemyAngle = atan2f(toEnemy.y, toEnemy.x);

        float angleStep = 2.0f * PI / debris_.debrisCount;
        float relativeAngle = NormalizeAngle(enemyAngle - debris_.rotationAngle);

        if (relativeAngle < 0) relativeAngle += 2.0f * PI;
        int centerIndex = (int)(relativeAngle / angleStep) % debris_.debrisCount;

        for (int offset = -2; offset <= 2; offset++) {
            int idx = (centerIndex + offset + debris_.debrisCount) % debris_.debrisCount;
            const auto& piece = debris_.pieces[idx];

            float r = debris_.currentRadius;
            if (debris_.state == DebrisState::Expanding || debris_.state == DebrisState::WaitMax) {
                float expandAmount = debris_.currentRadius - debris_.minRadius;
                r = debris_.minRadius + expandAmount * piece.speedScale + piece.distNoise;
            }
            else {
                r = debris_.currentRadius + piece.distNoise;
            }

            float a = piece.angleOffset + debris_.rotationAngle;

            Vector2 debrisPos = {
                player_.pos.x + cosf(a) * r,
                player_.pos.y + sinf(a) * r
            };

            float dist = Vector2::Length(e.pos - debrisPos);
            if (dist < debrisCollisionRadius + e.radius) {

                // --- ヒット時の処理 ---
                if (debris_.state == DebrisState::Contracting) {
                    // 攻撃
                    e.hp--;
                    StartShake(0.05f, 2.0f);
                    hitStopTimer_ = 0.02f;

                    Vector2 knockDir = Vector2::Normalize(e.pos - player_.pos);
                    float power = debris_.isCritical ? 1000.0f : 600.0f;
                    if (debris_.isCritical) e.hp -= 2;

                    e.knockbackVel = knockDir * power;
                    e.knockbackDuration = 0.3f;

                    if (e.hp <= 0) e.isAlive = false;
                    break;

                }
                else {
                    // 防御（押し出し）
                    Vector2 pushDir = Vector2::Normalize(e.pos - debrisPos);
                    e.pos = debrisPos + pushDir * (debrisCollisionRadius + e.radius);
                }
            }
        }
    }
}

void PrototypeSurvivalScene::SpawnEnemy() {
    for (auto& e : enemies_) {
        if (!e.isAlive) {
            e.isAlive = true;
            e.type = (rand() % 5 == 0) ? EnemyType::Tank : EnemyType::Normal;
            e.hp = (e.type == EnemyType::Tank) ? 5 : 1;
            e.radius = (e.type == EnemyType::Tank) ? 20.0f : 12.0f;
            e.knockbackDuration = 0.0f;
            e.knockbackVel = { 0,0 };

            float angle = (float)(rand() % 360) * PI / 180.0f;
            float spawnDist = sqrt(kWindowWidth * kWindowWidth + kWindowHeight * kWindowHeight) / 2.0f + 50.0f;
            e.pos = player_.pos + Vector2{ cosf(angle), sinf(angle) } * spawnDist;
            break;
        }
    }
}

void PrototypeSurvivalScene::StartShake(float duration, float power) {
    screenShakeTimer_ = duration;
    screenShakePower_ = power;
}

void PrototypeSurvivalScene::Draw() {
    Novice::DrawBox(0, 0, (int)kWindowWidth, (int)kWindowHeight, 0.0f,0x222233FF, kFillModeSolid);

    // 敵
    for (const auto& e : enemies_) {
        if (!e.isAlive) continue;
        unsigned int color = (e.type == EnemyType::Tank) ? COLOR_ENEMY_TANK : COLOR_ENEMY_NORMAL;
        if (e.knockbackDuration > 0) color = 0xFFFFFFFF;

        Novice::DrawBox(
            (int)(e.pos.x - e.radius + cameraOffset_.x),
            (int)(e.pos.y - e.radius + cameraOffset_.y),
            (int)(e.radius * 2), (int)(e.radius * 2),
            0.0f, color, kFillModeSolid
        );
    }

    // プレイヤー
    unsigned int pColor = player_.color;
    if (player_.invincibilityTimer > 0 && (int)(player_.invincibilityTimer * 20) % 2 == 0) {
        pColor = 0x8888FFFF;
    }
    Novice::DrawBox(
        (int)(player_.pos.x - player_.radius + cameraOffset_.x),
        (int)(player_.pos.y - player_.radius + cameraOffset_.y),
        (int)(player_.radius * 2), (int)(player_.radius * 2),
        0.0f, pColor, kFillModeSolid
    );

    // デブリ
    unsigned int debrisColor = COLOR_DEBRIS_SAFE;
    if (debris_.state == DebrisState::Expanding) debrisColor = COLOR_DEBRIS_EXPAND;
    if (debris_.state == DebrisState::WaitMax) debrisColor = debris_.isCritical ? 0xFFFFFFFF : COLOR_DEBRIS_EXPAND;
    if (debris_.state == DebrisState::Contracting) debrisColor = COLOR_DEBRIS_ATTACK;
    if (debris_.state == DebrisState::Cooldown) debrisColor = COLOR_DEBRIS_COOLDOWN;

    for (int i = 0; i < debris_.debrisCount; i++) {
        const auto& piece = debris_.pieces[i];

        float r = debris_.currentRadius;
        if (debris_.state == DebrisState::Expanding || debris_.state == DebrisState::WaitMax) {
            float expandAmount = debris_.currentRadius - debris_.minRadius;
            r = debris_.minRadius + expandAmount * piece.speedScale + piece.distNoise;
        }
        else {
            r = debris_.currentRadius + piece.distNoise;
        }

        float a = piece.angleOffset + debris_.rotationAngle;

        Vector2 debrisWorldPos = {
            player_.pos.x + cosf(a) * r,
            player_.pos.y + sinf(a) * r
        };

        float drawRot = a + piece.currentSelfAngle;

        Novice::DrawBox(
            (int)(debrisWorldPos.x - debris_.debrisSize / 2 + cameraOffset_.x),
            (int)(debrisWorldPos.y - debris_.debrisSize / 2 + cameraOffset_.y),
            (int)debris_.debrisSize, (int)debris_.debrisSize,
            drawRot, debrisColor, kFillModeSolid
        );
    }

    Novice::ScreenPrintf(10, 10, "HP: %d", player_.hp);
    Novice::ScreenPrintf(10, 30, "Radius: %.1f", debris_.currentRadius);
    if (debris_.isCritical) Novice::ScreenPrintf(10, 50, "CRITICAL READY!");
}