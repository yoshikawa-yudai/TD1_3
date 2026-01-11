#pragma once
#include "IGameScene.h"
#include "InputManager.h"
#include "Vector2.h"
#include <vector>
#include <memory>

// 前方宣言
class SceneManager;

/// <summary>
/// 「収縮と発散」プロトタイプシーン（IGameScene対応版）
/// </summary>
class PrototypeSurvivalScene : public IGameScene {
public:
    // SceneManagerを受け取るコンストラクタに変更
    PrototypeSurvivalScene(SceneManager& manager);
    ~PrototypeSurvivalScene() override;

    // IGameSceneのインターフェースに合わせる
    void Update(float deltaTime, const char* keys, const char* preKeys) override;
    void Draw() override;

private: // --- 内部クラス・構造体定義 ---

    // プレイヤー（コア）
    struct Proto_Player {
        Vector2 pos;
        float radius = 16.0f;
        int hp = 5;
        int maxHp = 5;
        float invincibilityTimer = 0.0f;
        unsigned int color = 0xFFFFFFFF;
    };

    // デブリの個体情報（バラつき用）
    struct DebrisPiece {
        float angleOffset; // 配置角度
        float distNoise;   // 半径の微妙なズレ
        float speedScale;  // 広がる速度の個体差
        float selfRotSpeed;// 自転速度
        float currentSelfAngle; // 現在の自転角度
    };

    // デブリ管理
    enum class DebrisState {
        Idle, Expanding, WaitMax, Contracting, Cooldown
    };

    struct Proto_DebrisManager {
        float currentRadius;
        float minRadius = 60.0f;
        float maxRadius = 250.0f;
        float rotationAngle = 0.0f; // 全体の公転

        DebrisState state = DebrisState::Idle;
        float cooldownTimer = 0.0f;
        bool isCritical = false;

        float expandSpeed = 300.0f;
        float contractSpeed = 1200.0f;
        float rotationSpeed = 2.0f;

        // 個々のデブリ管理用
        std::vector<DebrisPiece> pieces; // 個体配列
        int debrisCount = 64;
        float debrisSize = 12.0f;
    };

    // 敵
    enum class EnemyType { Normal, Tank };
    struct Proto_Enemy {
        Vector2 pos;
        Vector2 velocity;
        EnemyType type;
        int hp;
        float radius;
        bool isAlive = false;

        Vector2 knockbackVel = { 0,0 };
        float knockbackDuration = 0.0f;
    };

private: // --- メンバ変数 ---

    // シーン遷移等に使うため保持
    SceneManager* manager_;

    std::unique_ptr<InputManager> input_;
    Proto_Player player_;
    Proto_DebrisManager debris_;
    std::vector<Proto_Enemy> enemies_;

    float enemySpawnTimer_ = 0.0f;
    float hitStopTimer_ = 0.0f;
    float screenShakeTimer_ = 0.0f;
    float screenShakePower_ = 0.0f;
    Vector2 cameraOffset_ = { 0,0 };

private: // --- ヘルパー関数 ---

    void UpdatePlayer(float dt);
    void UpdateDebris(float dt);
    void UpdateEnemies(float dt);
    void CheckCollisions();

    void SpawnEnemy();
    void StartShake(float duration, float power);

    // 角度を正規化する関数 (-PI ~ PI)
    float NormalizeAngle(float angle);
};