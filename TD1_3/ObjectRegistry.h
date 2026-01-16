#pragma once
#include <string>
#include <vector>
#include "TextureManager.h"

/// <summary>
/// ゲームオブジェクトタイプ定義
/// </summary>
struct ObjectTypeDefinition {
    int id;                 // オブジェクトタイプID（100=Player, 101=Enemy等）
    std::string name;       // エディタ表示名
    TextureId iconTexture;  // エディタで表示するアイコン
    std::string tag;        // デフォルトタグ
};

/// <summary>
/// ゲームオブジェクトタイプのレジストリ
/// </summary>
class ObjectRegistry {
public:
    static const std::vector<ObjectTypeDefinition>& GetAllObjectTypes() { return objectTypes_; }

    static const ObjectTypeDefinition* GetObjectType(int id) {
        for (const auto& obj : objectTypes_) {
            if (obj.id == id) return &obj;
        }
        return nullptr;
    }

    static void Initialize() {
        objectTypes_.clear();

        // --- Player ---
        objectTypes_.push_back({
            100, "PlayerStart", TextureId::PlayerAnimeNormal, "player"
            });

        // --- Enemies ---
        // objectTypes_.push_back({
        //     101, "Enemy_Normal", TextureId::Enemy_Normal, "enemy"
        // });
        // objectTypes_.push_back({
        //     102, "Enemy_Flying", TextureId::Enemy_Flying, "enemy"
        // });

        // --- Items ---
        // objectTypes_.push_back({
        //     103, "Item_Coin", TextureId::Item_Coin, "item"
        // });

        // --- Other ---
        // objectTypes_.push_back({
        //     200, "Checkpoint", TextureId::Checkpoint, "checkpoint"
        // });
    }

private:
    static std::vector<ObjectTypeDefinition> objectTypes_;
};

inline std::vector<ObjectTypeDefinition> ObjectRegistry::objectTypes_;