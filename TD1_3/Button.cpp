#include "Button.h"
#include "Easing.h"
#include <Novice.h>
#include <algorithm>

Button::Button(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback)
	: position_(position),
	size_(size),
	label_(label),
	callback_(callback),
	isImageButton_(false) {
}

Button::Button(const Vector2& position, const Vector2& size, int normalTexture, int selectedTexture, std::function<void()> callback)
	: position_(position),
	callback_(callback),
	normalTexture_(normalTexture),
	selectedTexture_(selectedTexture),
	isImageButton_(true) {

	size;

	// テクスチャサイズを自動取得
	int width = 0;
	int height = 0;
	Novice::GetTextureSize(normalTexture, &width, &height);

	// メンバの size_ をテクスチャ実寸に
	size_ = size;

	/*DrawComponent2D(int graphHandle, int divX, int divY, int totalFrames,
		float speed, bool isLoop = true);*/

	// 通常時のDrawComponent2D初期化（size_.x / size_.y を使う）
	drawCompNormal_ = DrawComponent2D(
		normalTexture_,
		1,1,1,1.0f,false
	);
	drawCompNormal_.SetPosition(position_);
	drawCompNormal_.SetScale(size_);

	// 選択時のDrawComponent2D初期化
	drawCompSelected_ = DrawComponent2D(
		normalTexture_,
		1, 1, 1, 1.0f, false
	);
	drawCompNormal_.SetPosition(position_);
	drawCompNormal_.SetScale(size_);

	// アニメーション停止（静止画として使用）
	drawCompNormal_.StopAnimation();
	drawCompSelected_.StopAnimation();
}

void Button::Update(float deltaTime, bool isSelected) {
	isSelected_ = isSelected;

	// イージングのターゲット値を設定
	float target = isSelected_ ? 1.0f : 0.0f;

	// イージングでスケールを変更
	easeT_ += (target - easeT_) * std::clamp(easeSpeed_ * deltaTime, 0.0f, 1.0f);

	// イージングを適用してスケールを計算
	float eased = Easing::EaseOutQuad(easeT_);
	float scale = std::lerp(scaleMin_, scaleMax_, eased);

	// 画像ボタンの場合、DrawComponentのスケールを更新
	if (isImageButton_) {
		drawCompNormal_.SetScale({scale, -scale});
		drawCompNormal_.SetPosition(position_);
		drawCompSelected_.SetScale({ scale, -scale });
		drawCompSelected_.SetPosition(position_);
	}
}

void Button::Draw(int textureHandle, FontAtlas* font, TextRenderer* textRenderer) {
	if (isImageButton_) {
		// 画像ボタンの描画：選択状態に応じて切り替え
		if (isSelected_) {
			drawCompSelected_.DrawScreen();
		}
		else {
			// 半透明描画をDrawComponent側に設定し描画後に元に戻す
			drawCompNormal_.SetBaseColor(0xFFFFFF99);
			drawCompNormal_.DrawScreen();
			drawCompNormal_.SetBaseColor(0xFFFFFFFF);
		}
	}
	else {
		// 既存のテキストボタン描画処理
		textureHandle; // 未使用パラメータ回避

		uint32_t fillColor = isSelected_ ? colorSelected_ : colorNormal_;

		Vector2 scale = { easeT_, easeT_ };
		float w = size_.x * std::lerp(scaleMin_, scaleMax_, Easing::EaseOutQuad(easeT_));
		float h = size_.y * std::lerp(scaleMin_, scaleMax_, Easing::EaseOutQuad(easeT_));

		float left = position_.x - w * anchor_.x;
		float top = position_.y - h * anchor_.y;

		Novice::DrawBox(
			static_cast<int>(left),
			static_cast<int>(top),
			static_cast<int>(w),
			static_cast<int>(h),
			0.0f,
			fillColor,
			kFillModeSolid
		);

		uint32_t frameColor = isSelected_ ? colorFrameSelected_ : colorFrame_;
		Novice::DrawBox(
			static_cast<int>(left),
			static_cast<int>(top),
			static_cast<int>(w),
			static_cast<int>(h),
			0.0f,
			frameColor,
			kFillModeWireFrame
		);

		if (font && textRenderer) {
			float labelScale = isSelected_ ? textScaleSelected_ : textScale_;
			uint32_t textColor = isSelected_ ? colorTextSelected_ : colorText_;

			int textWidth = textRenderer->MeasureWidth(label_.c_str(), labelScale);
			int textHeight = static_cast<int>(font->GetLineHeight() * labelScale);

			int textX = static_cast<int>(left + (w - textWidth) * 0.5f);
			int textY = static_cast<int>(top + (h - textHeight) * 0.5f);

			textRenderer->DrawTextLabel(textX, textY, label_.c_str(), textColor, labelScale);
		}
	}
}

void Button::Execute() {
	if (callback_) {
		callback_();
	}
}