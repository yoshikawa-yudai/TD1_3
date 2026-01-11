#pragma once
#include <list>
#include <memory>
#include <vector>
#include "GameObject2D.h"
#include "Camera2D.h"

// 前方宣言
class SurvivalPlayer;
class DebrisController;
class SurvivalEnemy;

/// <summary>
/// サバイバルゲーム用オブジェクトマネージャー
/// </summary>
class SurvivalGameObjectManager {
public:
    SurvivalGameObjectManager();
    ~SurvivalGameObjectManager();

    // 更新（全オブジェクトの更新と削除処理）
    void Update(float deltaTime);

    // 描画（全オブジェクトの描画）
    void Draw(const Camera2D& camera);

    // オブジェクト登録
    void AddObject(const std::shared_ptr<GameObject2D>& obj, const std::string& tag);

    // 特定オブジェクトへのアクセサ（判定やカメラ制御で使用）
    void SetPlayer(std::shared_ptr<SurvivalPlayer> player) { player_ = player; }
    std::shared_ptr<SurvivalPlayer> GetPlayer() const { return player_; }

    void SetDebrisController(std::shared_ptr<DebrisController> debris) { debrisController_ = debris; }
    std::shared_ptr<DebrisController> GetDebrisController() const { return debrisController_; }

    // 敵のリストを取得（デブリ側から参照するため）
    const std::list<std::shared_ptr<SurvivalEnemy>>& GetEnemies() const { return enemies_; }

    // 全消去（リセット用）
    void Clear();

    unsigned int GetObjectsSize() {
        return static_cast<unsigned int>(objects_.size());
    }

private:
    // 全オブジェクトリスト（所有権を持つ）
    std::list<std::shared_ptr<GameObject2D>> objects_;

    // 参照用キャッシュ（objects_の中身を指す弱い参照）
    // 毎回キャストして探すと重いため、分けて保持する
    std::shared_ptr<SurvivalPlayer> player_;
    std::shared_ptr<DebrisController> debrisController_;
    std::list<std::shared_ptr<SurvivalEnemy>> enemies_;

    // 衝突判定ロジック（内部で呼ぶ）
    void CheckCollisions();
};