#pragma once
#include "GameObject2D.h"
#include "DrawComponent2D.h"
#include "InputManager.h"

// 共通定数や前方宣言
class SurvivalGameObjectManager;

// ==========================================
// プレイヤー (Core)
// ==========================================
class SurvivalPlayer : public GameObject2D {
public:
    SurvivalPlayer(InputManager* input);
    void Update(float dt) override;

    // 固有メソッド
    void OnDamage();
    int GetHP() const { return hp_; }
    float GetRadius() const { return radius_; }

    // 描画コンポーネントへのアクセサ
    DrawComponent2D* GetDrawComp() const { return drawComp_; }

private:
    InputManager* input_;

    float speed_ = 300.0f;
    float radius_ = 16.0f;
    int hp_ = 5;
    float invincibilityTimer_ = 0.0f;
};

// ==========================================
// 敵 (Enemy)
// ==========================================
enum class EnemyType { Normal, Tank };

class SurvivalEnemy : public GameObject2D {
public:
    SurvivalEnemy(Vector2 startPos, EnemyType type, std::shared_ptr<SurvivalPlayer> target);
    void Update(float dt) override;

    // 固有メソッド
    void OnHit(int damage, Vector2 knockbackDir, float knockbackPower);
    void PushBack(Vector2 dir, float dist); // 押し出し処理

    EnemyType GetType() const { return type_; }
    float GetRadius() const { return radius_; }
    bool IsInvincible() const { return hitInvincibility_ > 0.0f; }

private:
    std::shared_ptr<SurvivalPlayer> target_; // 追尾対象

    EnemyType type_;
    int hp_;
    float radius_;

    // ノックバック制御
    Vector2 knockbackVel_ = { 0,0 };
    float knockbackDuration_ = 0.0f;
    float hitInvincibility_ = 0.0f;
};

// ==========================================
// がれき1粒 (DebrisPiece)
// ==========================================
class DebrisPiece : public GameObject2D {
public:
    DebrisPiece(int index, int totalCount, std::shared_ptr<SurvivalPlayer> anchor);
    void Update(float dt) override;

    // コントローラーから制御されるパラメータ
    void SetStateInfo(float currentRadius, float rotationAngle, float expandSpeedScale);

    // 固有メソッド
    // 「本来あるべき位置」と「現在位置」のズレを利用して慣性を表現
    Vector2 GetActualPosition() const { return transform_.translate; }

private:
    std::shared_ptr<SurvivalPlayer> anchor_; // 中心点（プレイヤー）

    int index_;
    int totalCount_;

    // 個体差パラメータ
    float angleOffset_;
    float distNoise_;
    float selfRotSpeed_;
    float currentSelfRot_ = 0.0f;

    // 慣性制御用
    Vector2 targetPos_ = { 0,0 }; // 本来あるべき場所
};

// ==========================================
// がれき管理者 (DebrisController)
// ==========================================
// これ自体は描画を持たず、DebrisPieceを生成して操る
class DebrisController : public GameObject2D {
public:
    DebrisController(SurvivalGameObjectManager* manager, InputManager* input, std::shared_ptr<SurvivalPlayer> anchor);
    void Update(float dt) override;

    // 状態アクセサ
    float GetCurrentRadius() const { return currentRadius_; }
    float GetMaxRadius() const { return maxRadius_; }
    bool IsExpanding() const;   // 拡大中か？
    bool IsContracting() const; // 収縮（攻撃）中か？
    bool IsCritical() const { return isCritical_; }

    // すべてのDebrisPieceへのconst参照を返す
    const std::vector<std::shared_ptr<DebrisPiece>>& GetPieces() const { return pieces_; }

private:
    SurvivalGameObjectManager* manager_;
    InputManager* input_;
    std::shared_ptr<SurvivalPlayer> anchor_;

    // パラメータ
    float minRadius_ = 60.0f;
    float maxRadius_ = 350.0f;
    float currentRadius_;

    float rotationAngle_ = 0.0f;
    float currentRotationSpeed_ = 0.0f;

    // ステート管理
    enum class State { Idle, Expanding, WaitMax, Contracting, Cooldown };
    State state_ = State::Idle;

    float cooldownTimer_ = 0.0f;
    bool isCritical_ = false;

    // Pieceへの参照（一括操作用）
    std::vector<std::shared_ptr<DebrisPiece>> pieces_;
};