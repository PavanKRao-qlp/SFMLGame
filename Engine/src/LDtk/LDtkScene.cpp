#include "LDtk/LDtkScene.h"

#include <filesystem>

#include "Asset/AssetManager.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "LDtk/LDtkEntityComponent.h"
#include "Service/ServiceLocator.h"
#include "Thread/Mutex.h"

#include "LDtkLoader/Project.hpp"

namespace Umbra {

namespace {

    // Resolve a tileset-relative path (stored inside the .ldtk JSON) to a
    // normalised path that AssetManager can open.
    String ResolveTilesetPath(const String& _ldtkPath, const std::string& _relPath) {
        namespace fs = std::filesystem;
        fs::path ldtkDir = fs::path(_ldtkPath).parent_path();
        fs::path full    = (ldtkDir / _relPath).lexically_normal();
        String   str     = full.string();
        std::replace(str.begin(), str.end(), '\\', '/');
        return str;
    }

    // Serialise the scalar LDtk custom fields of an entity to a flat string map.
    // Complex types (Point, Color, arrays, EntityRef) are skipped.
    UMap<String, String> SerialiseFields(const ldtk::Entity& _entity) {
        UMap<String, String> out;
        for (const auto& def : _entity.allFields()) {
            const auto& name = def.name;
            String      value;
            switch (def.type) {
                case ldtk::FieldType::Int: {
                    auto& f = _entity.getField<int>(name);
                    value   = f.is_null() ? "" : std::to_string(f.value());
                    break;
                }
                case ldtk::FieldType::Float: {
                    auto& f = _entity.getField<float>(name);
                    value   = f.is_null() ? "" : std::to_string(f.value());
                    break;
                }
                case ldtk::FieldType::Bool: {
                    auto& f = _entity.getField<bool>(name);
                    value   = f.is_null() ? "" : (f.value() ? "true" : "false");
                    break;
                }
                case ldtk::FieldType::String:
                case ldtk::FieldType::FilePath: {
                    auto& f = _entity.getField<std::string>(name);
                    value   = f.is_null() ? "" : f.value();
                    break;
                }
                case ldtk::FieldType::Enum: {
                    auto& f = _entity.getField<ldtk::EnumValue>(name);
                    value   = f.is_null() ? "" : f.value().name;
                    break;
                }
                default:
                    continue; // skip complex/array types
            }
            out[name] = std::move(value);
        }
        return out;
    }

    // Parse one LDtk world into LDtkLoadData, appending to tiles/entities/paths.
    void ParseWorld(const ldtk::World&       _world,
                    const String&             _ldtkPath,
                    const String&             _levelName,
                    LDtkLoadData&             _data) {
        for (const auto& level : _world.allLevels()) {
            if (!_levelName.empty() && level.name != _levelName) {
                continue;
            }

            const auto& layers     = level.allLayers();
            int         totalLayers = static_cast<int>(layers.size());
            int         layerIndex  = 0;

            for (const auto& layer : layers) {
                int  zOrder = (totalLayers - layerIndex) * 10;
                auto type   = layer.getType();
                ++layerIndex;

                if (type == ldtk::LayerType::Tiles || type == ldtk::LayerType::AutoLayer) {
                    if (!layer.hasTileset()) continue;
                    const auto& tileset = layer.getTileset();
                    if (tileset.path.empty()) continue;

                    String fullPath = ResolveTilesetPath(_ldtkPath, tileset.path);

                    // Register unique tileset path
                    bool found = false;
                    for (const auto& p : _data.uniqueTilesetPaths) {
                        if (p == fullPath) { found = true; break; }
                    }
                    if (!found) _data.uniqueTilesetPaths.push_back(fullPath);

                    int   cellSize = layer.getCellSize();
                    float tsW      = static_cast<float>(tileset.texture_size.x);
                    float tsH      = static_cast<float>(tileset.texture_size.y);

                    for (const auto& tile : layer.allTiles()) {
                        auto wp = tile.getWorldPosition();
                        auto tr = tile.getTextureRect();

                        LDtkLoadData::TileEntry entry;
                        entry.x           = static_cast<float>(wp.x) + cellSize * 0.5f;
                        entry.y           = static_cast<float>(wp.y) + cellSize * 0.5f;
                        entry.width       = static_cast<float>(cellSize);
                        entry.height      = static_cast<float>(cellSize);
                        entry.tilesetPath = fullPath;
                        entry.uvX         = static_cast<float>(tr.x) / tsW;
                        entry.uvY         = static_cast<float>(tr.y) / tsH;
                        entry.uvW         = static_cast<float>(tr.width) / tsW;
                        entry.uvH         = static_cast<float>(tr.height) / tsH;
                        entry.zOrder      = zOrder;
                        _data.tiles.push_back(std::move(entry));
                    }

                } else if (type == ldtk::LayerType::Entities) {
                    for (const auto& entity : layer.allEntities()) {
                        auto wp = entity.getWorldPosition();
                        auto sz = entity.getSize();
                        auto pv = entity.getPivot();

                        LDtkLoadData::EntityEntry entry;
                        entry.x        = static_cast<float>(wp.x) + sz.x * (0.5f - pv.x);
                        entry.y        = static_cast<float>(wp.y) + sz.y * (0.5f - pv.y);
                        entry.width    = static_cast<float>(sz.x);
                        entry.height   = static_cast<float>(sz.y);
                        entry.typeName = entity.getName();
                        entry.fields   = SerialiseFields(entity);
                        entry.zOrder   = zOrder;
                        _data.entities.push_back(std::move(entry));
                    }
                }
            }
        }
    }

} // namespace

// ============================================================
// LDtkScene
// ============================================================

LDtkScene::LDtkScene(const String& _ldtkPath, const String& _levelName)
    : mLdtkPath(_ldtkPath)
    , mLevelName(_levelName) {}

void LDtkScene::Initialize() {
    // Register LDtkEntityComponent with this world's ECS.
    GetWorld()->RegisterComponent<LDtkEntityComponent>();

    mLoadData = std::make_shared<LDtkLoadData>();

    JobSystem* js = ServiceLocator::GetJobSystem();

    if (!js) {
        // --- Fallback: run everything synchronously (no JobSystem) ---
        ldtk::Project project;
        project.loadFromFile(mLdtkPath);
        for (const auto& world : project.allWorlds()) {
            ParseWorld(world, mLdtkPath, mLevelName, *mLoadData);
        }
        for (const auto& path : mLoadData->uniqueTilesetPaths) {
            mLoadData->textures[path] = AssetManager::GetInstance()->GetTexture(path);
        }
        mLoadData->bReady.Store(true, EMemoryOrder::Release);
        return;
    }

    // --- Async path ---
    SharedPtr<LDtkLoadData> data      = mLoadData;
    String                  ldtkPath  = mLdtkPath;
    String                  levelName = mLevelName;

    // Job A: parse the .ldtk file on a single worker thread.
    JobHandle hA = js->Submit([data, ldtkPath, levelName]() {
        ldtk::Project project;
        project.loadFromFile(ldtkPath);
        for (const auto& world : project.allWorlds()) {
            ParseWorld(world, ldtkPath, levelName, *data);
        }
    });

    // Job B+C: after parsing completes, load all tileset textures in parallel,
    // then signal the main thread that spawning can begin.
    js->SubmitAfter(hA, [data, js]() {
        uint32 count = static_cast<uint32>(data->uniqueTilesetPaths.size());
        if (count > 0) {
            // Job B: ParallelFor — each item loads one texture (AssetManager is thread-safe).
            js->ParallelFor(count, [data](uint32 _i) {
                const String& path = data->uniqueTilesetPaths[_i];
                SharedPtr<Texture> tex = AssetManager::GetInstance()->GetTexture(path);
                LockGuard<Mutex> lock(data->texturesMutex);
                data->textures[path] = tex;
            }).Wait();
        }
        // Job C: signal ready (Release so all prior writes are visible).
        data->bReady.Store(true, EMemoryOrder::Release);
    });
}

void LDtkScene::OnUpdate() {
    if (mEntitiesSpawned) return;
    if (!mLoadData || !mLoadData->bReady.Load(EMemoryOrder::Acquire)) return;
    SpawnECSEntities();
    mEntitiesSpawned = true;
}

void LDtkScene::SpawnECSEntities() {
    World*            world = GetWorld();
    const LDtkLoadData& data = *mLoadData;

    // --- Tile-layer entities ---
    for (const auto& tile : data.tiles) {
        auto it = data.textures.find(tile.tilesetPath);
        if (it == data.textures.end() || !it->second) continue;

        EntityID e = world->CreateEntity();
        world->AddComponent<TransformComponent>(
            e,
            TransformComponent(
                Math::Vector2f(tile.x, tile.y),
                Math::Vector2f(tile.width, tile.height)));

        // SpriteComponent takes SharedPtr<Texture>& — bind from a named copy.
        SharedPtr<Texture> tex = it->second;
        SpriteComponent    sprite(tex, tile.zOrder);
        sprite.uvRect = FloatRect(tile.uvX, tile.uvY, tile.uvW, tile.uvH);
        world->AddComponent<SpriteComponent>(e, sprite);
    }

    // --- Entity-layer entities ---
    for (const auto& ent : data.entities) {
        EntityID e = world->CreateEntity();
        world->AddComponent<TransformComponent>(
            e,
            TransformComponent(
                Math::Vector2f(ent.x, ent.y),
                Math::Vector2f(ent.width, ent.height)));

        world->AddTag(e, ent.typeName);

        LDtkEntityComponent ldtkComp;
        ldtkComp.typeName = ent.typeName;
        ldtkComp.fields   = ent.fields;
        world->AddComponent<LDtkEntityComponent>(e, ldtkComp);
    }
}

SharedPtr<Scene> LDtkScene::InstantiateCopy() {
    return std::make_shared<LDtkScene>(mLdtkPath, mLevelName);
}

} // namespace Umbra
