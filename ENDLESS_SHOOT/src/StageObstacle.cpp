#include "StageObstacle.h"
#include "world.h"
#include "Transform.h"
#include "Collider.h"
#include "MeshRenderer.h"
#include "ShaderList.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

namespace
{
    // モデルの判定に使用する定数
    constexpr float WALL_ASPECT_RATIO = 2.0f;
    constexpr float PILLAR_ASPECT_RATIO = 1.5f;

    // モデルの中心と大きさ
    struct ModelBounds
    {
        Vector3 center;
        Vector3 size;
    };

    const ModelBounds WALL_BOUNDS = {
        Vector3(0.0f, 2.0f, -0.042693555f),
        Vector3(6.0f, 4.0f, 2.354689121f)
    };
    const ModelBounds TOWER_BOUNDS = {
        Vector3(0.5f, 2.0f, 0.0f),
        Vector3(2.0f, 4.0f, 4.0f)
    };

    struct ModelAsset
    {
        std::shared_ptr<Model> model;
        Vector3 center;
        Vector3 size;
    };

    ModelAsset LoadModel(bool isWall)
    {
        static std::weak_ptr<Model> wallCache;
        static std::weak_ptr<Model> towerCache;

        // モデルと寸法を選択
        std::weak_ptr<Model>& cache = isWall ? wallCache : towerCache;
        const ModelBounds& bounds = isWall ? WALL_BOUNDS : TOWER_BOUNDS;

        if (auto model = cache.lock())
        {
            return { model, bounds.center, bounds.size };   // モデルが存在したら返す
        }

        // ロードする
        const char* path = isWall
            ? "Assets/Model/building/Wall6x_01.fbx"
            : "Assets/Model/building/SkyTower2.fbx";
        auto model = std::make_shared<Model>();
        if (!model->Load(path))
        {
            throw std::runtime_error(std::string("Cannot load obstacle model: ") + path);
        }
        model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
        model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

        cache = model;
        return { model, bounds.center, bounds.size };
    }

    // TypeがAutoの時にTypeを判定する
    StageObstacle::Type ResolveType(const Vector3& size, StageObstacle::Type type)
    {
        if (type != StageObstacle::Type::Auto)
        {
            return type;
        }
        const float longestSide = (std::max)(size.x, size.z);
        const float shortestSide = (std::min)(size.x, size.z);
        if (longestSide >= shortestSide * WALL_ASPECT_RATIO)
        {
            return StageObstacle::Type::Wall;   // x ,zの比率を見て壁かどうか判定
        }
        return size.y >= longestSide * PILLAR_ASPECT_RATIO              // yの比率見て長かったら柱
            ? StageObstacle::Type::Pillar : StageObstacle::Type::Block;
    }

    void PlaceModel(ECS::World& world, const ModelAsset& asset,
        const Vector3& center, const Vector3& size, const Quaternion& rotation)
    {
        Transform transform;
        transform.scale = Vector3(size.x / asset.size.x,size.y / asset.size.y, size.z / asset.size.z);
        transform.rotation = rotation;

        // スケールを加味して中心座標のズレを補正する
        const Vector3 scaledCenter(asset.center.x * transform.scale.x,asset.center.y * transform.scale.y,asset.center.z * transform.scale.z);
        
        transform.position = center - rotation * scaledCenter;

        const ECS::EntityID entity = world.CreateEntity();
        world.AddComponent<Transform>(entity, transform);
        world.AddComponent<MeshRenderer>(entity, MeshRenderer(asset.model));
    }
}

ECS::EntityID StageObstacle::Create(ECS::World& world, const Vector3& center,
    const Vector3& size, Type type)
{
    for (int axis = 0; axis < 3; ++axis)
    {
        if (!std::isfinite(center[axis]) || !std::isfinite(size[axis]) || size[axis] <= 0.0f)   // 値が正しいかチェック
        {
            throw std::invalid_argument("Obstacle center must be finite and size must be positive.");
        }
    }
    type = ResolveType(size, type);
    const bool isWall = type == Type::Wall || type == Type::Cover;
    const ModelAsset asset = LoadModel(isWall);

    // コライダー用のエンティティを作る
    const ECS::EntityID colliderEntity = world.CreateEntity();
    Transform colliderTransform;
    colliderTransform.position = center;
    world.AddComponent<Transform>(colliderEntity, colliderTransform);
    BoxCollider collider(Vector3(0.0f, 0.0f, 0.0f), size, Quaternion::Identity());
    collider.isStatic = true;
    world.AddComponent<BoxCollider>(colliderEntity, collider);

    if (!isWall)
    {
        PlaceModel(world, asset, center, size, Quaternion::Identity());     // Wall出ないならモデル一個表示して終了
        return colliderEntity;
    }

    // 壁の向きと長さを測る
    const bool isAlongZ = size.z > size.x;
    const float length = isAlongZ ? size.z : size.x;
    const float depth = isAlongZ ? size.x : size.z;

    const float preferredLength = size.y * asset.size.x / asset.size.y;     // モデル一個の長さ
    const int tileCount = (std::max)(1, static_cast<int>(std::ceil(length / preferredLength))); // モデルを置く個数(切り上げで一個以上にする)
    const float tileLength = length / static_cast<float>(tileCount);    // 個数から計算する実際のモデルの長さ

    const Quaternion rotation = isAlongZ ? Quaternion::FromRotation(0.0f, 90.0f, 0.0f) : Quaternion::Identity();    // 必要なら回転

    for (int tileIndex = 0; tileIndex < tileCount; ++tileIndex)
    {
        Vector3 tileCenter = center;
        const float offset = -length * 0.5f + (static_cast<float>(tileIndex) + 0.5f) * tileLength;  // 壁の中心から各モデルの中心へのオフセット

        if (isAlongZ)
        {
            tileCenter.z += offset;
        }
        else
        {
            tileCenter.x += offset;
        }
        PlaceModel(world, asset, tileCenter, Vector3(tileLength, size.y, depth), rotation);
    }
    return colliderEntity;
}
