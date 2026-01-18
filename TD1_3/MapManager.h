#pragma once
#include <vector>
#include <memory>
#include "TileInstance.h"
#include "MapData.h"
#include "Camera2D.h"

class MapManager {
public:
    MapManager() = default;
    ~MapManager() = default;

    // マップデータから演出用タイル(TileInstance)を生成
    void Initialize();

    // カメラ範囲に基づいた更新（カリング）
    void Update(float deltaTime, Camera2D& camera);

    // 描画
    void Draw(const Camera2D& camera);

    // 特定座標のタイルに干渉（桜井氏流：手応えの演出用フック）
    void InteractionTile(const Vector2& worldPos);

private:
    std::vector<std::unique_ptr<TileInstance>> dynamicTiles_;
};