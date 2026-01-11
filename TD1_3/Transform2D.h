#pragma once
#include "Matrix3x3.h"
#include "Affine2D.h"

// #include "GameObject.h" // 循環参照を避けるためGameObject* parent_は前方宣言で

struct Transform2D {
	// ローカル座標
	Vector2 translate = { 0.0f, 0.0f };
	Vector2 scale = { 1.0f, 1.0f };
	float rotation = 0.0f; // in radian

	// ワールド座標 (計算結果)
	Matrix3x3 worldMatrix;

	// World行列を再計算する関数
	void CalculateWorldMatrix() {
		worldMatrix = AffineMatrix2D::MakeAffine(scale, rotation, translate);
	}
};