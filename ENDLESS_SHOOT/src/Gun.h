/*****************************************************************//**
 * @file   Gun.h
 * @brief  銃の振る舞いを制御するスクリプトクラス
 *
 * @author 
 * @date   
 *********************************************************************/
#pragma once

#include "NativeScript.h"
#include "Vector.h"
#include "Audio.h"
#include "EffekseerManager.h"

/**
 * @brief 銃クラス
 */
class Gun : public NativeScript
{
public:
	/**
	 * @brief コンストラクタ
	 * @param _playerId 銃を所持するプレイヤーのEntityID
	 */
	Gun(ECS::EntityID _playerId);

	/**
	 * @brief デストラクタ
	 */
	~Gun() = default;

	/**
	 * @brief 初期化処理
	 */
	void OnCreate() override;

	/**
	 * @brief 更新処理
	 */
	void Update() override;

	/**
	 * @brief 後更新処理
	 */
	void LateUpdate() override;

	/**
	 * @brief 破棄時処理
	 */
	void OnDestroy() override;

	/**
	 * @brief ヒットマーカーを表示する
	 */
	void ShowHitMarker();

private:

	void Reload();
	void Shoot();

	/** @brief カメラのEntityID */
	ECS::EntityID cameraId;

	/** @brief プレイヤーから銃への座標 */
	Vector3 offset;

	/** @brief 発砲音のハンドル */
	soundHandle shootSE;

	// マガジン関連
    float magazineSize;
    float currentAmmo;
    float reloadTime;
    float reloadTimer;
    bool isReloading;
	soundHandle reloadSE;
	soundHandle hitSE;
	ECS::EntityID ammoBarId;
	float fireRate;
	float fireTimer;

	/** @brief 現在の反動オフセット */
	Vector3 currentRecoil;

	/** @brief ヒットマーカーUIのID */
	ECS::EntityID hitMarkerId;
	/** @brief ヒットマーカーの現在の透明度 */
	float hitMarkerAlpha;
};
