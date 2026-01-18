#include "MapManager.h"
#include "TileRegistry.h"
#include "WindowSize.h"

void MapManager::Initialize() {
    dynamicTiles_.clear();
    auto& mapData = MapData::GetInstance();
    float tileSize = mapData.GetTileSize();

    // 全レイヤーを走査してComponent指定のタイルを探す
    std::vector<TileLayer> layers = { TileLayer::Decoration, TileLayer::Block };

    for (auto layer : layers) {
        for (int y = 0; y < mapData.GetHeight(); ++y) {
            for (int x = 0; x < mapData.GetWidth(); ++x) {
                int id = mapData.GetTile(x, y, layer);
                if (id == 0) continue;

                const TileDefinition* def = TileRegistry::GetTile(id);
                if (def && def->renderMode == RenderMode::Component) {
                    // タイルの中心座標を計算
                    Vector2 worldPos = {
                        x * tileSize + tileSize * 0.5f,
                        y * tileSize + tileSize * 0.5f
                    };
                    dynamicTiles_.push_back(std::make_unique<TileInstance>(*def, worldPos));
                }
            }
        }
    }
}

void MapManager::Update(float deltaTime, Camera2D& camera) {
    // カメラ表示範囲より少し広い矩形（アクティブエリア）を計算
    float margin = 128.0f;
    Vector2 camPos = camera.GetPosition();
    float hw = (kWindowWidth * (1.0f / camera.GetZoom())) * 0.5f + margin;
    float hh = (kWindowHeight * (1.0f / camera.GetZoom())) * 0.5f + margin;

    for (auto& tile : dynamicTiles_) {
        Vector2 tPos = tile->GetWorldPos();
        bool inRange = (tPos.x > camPos.x - hw && tPos.x < camPos.x + hw &&
            tPos.y > camPos.y - hh && tPos.y < camPos.y + hh);

        tile->Update(deltaTime, inRange);
    }
}

void MapManager::Draw(const Camera2D& camera) {
    // 描画自体はTileInstance側でDrawComponent2Dを介して行う
    for (auto& tile : dynamicTiles_) {
        tile->Draw(camera);
    }
}

void MapManager::InteractionTile(const Vector2& worldPos) {
    //「手触り」：攻撃が当たった場所などのタイルを揺らす
    float range = 32.0f; // 判定半径
    for (auto& tile : dynamicTiles_) {
        Vector2 diff = { tile->GetWorldPos().x - worldPos.x, tile->GetWorldPos().y - worldPos.y };
        if (diff.x * diff.x + diff.y * diff.y < range * range) {
            tile->OnHit();
        }
    }
}