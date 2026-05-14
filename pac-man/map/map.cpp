#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include <SFML/Graphics/Sprite.hpp>

#include <memory>
#include <fstream>
#include <algorithm>

#include "map.h"
#include "object/object.h"
#include "object/pac-man.h"
#include "object/layer.h"

bool Map::loadFromFile(const std::string &filename)
{
    std::ifstream ifs(filename);

    tilesets.clear();
    animations.clear();
    objects.clear();

    if (!ifs.is_open())
    {
        std::cout << "Could not open file: " << filename << std::endl;
        return false;
    }

    rapidjson::Document root;
    rapidjson::IStreamWrapper isw(ifs);
    root.ParseStream(isw);

    if (root.HasParseError() || !root.IsObject())
    {
        std::cout << "JSON parse error!" << std::endl;
        return false;
    }

    if (root.HasMember("width"))
    {
        width = root["width"].GetInt();
    }
    else
    {
        width = root["layers"][0]["width"].GetInt();
    }

    if (root.HasMember("height"))
    {
        height = root["height"].GetInt();
    }
    else
    {
        height = root["layers"][0]["height"].GetInt();
    }

    if (root.HasMember("tilewidth"))
    {
        tileWidth = root["tilewidth"].GetInt();
    }
    else
    {
        tileWidth = 25;
    }

    if (root.HasMember("tileheight"))
    {
        tileHeight = root["tileheight"].GetInt();
    }
    else
    {
        tileHeight = 25;
    }

    for (auto &layer: root["layers"].GetArray())
    {
        std::string type = layer["type"].GetString();
        std::cout << "Loading layer: " << layer["name"].GetString() << std::endl;

        if (type == "tilelayer")
        {
            loadTileLayer(layer);
        }
        else
        {
            loadObjectLayer(layer);
        }
    }

    std::cout << "Map loaded successfully" << std::endl;
    return true;
}

std::vector<AnimationFrame*> *Map::getAnimation(unsigned int gid)
{
    auto animationIt = animations.find(gid);

    if (animationIt != animations.end())
    {
        return animationIt->second;
    }

    return nullptr;
}

std::tuple<const sf::Texture&, sf::Rect<int>> Map::getSpriteTextureFromGid(unsigned int gid, int frame)
{
    unsigned int flipFlags = gid >> 29;

    gid &= ~(0b111 << 29);

    auto animationIt = animations.find(gid);

    if (animationIt != animations.end())
    {
        auto animation = *animationIt->second;

        if (frame >= 0 && frame < static_cast<int>(animation.size()))
        {
            gid = animation[frame]->gid;
        }
    }

    auto tileset = *std::find_if(
        tilesets.rbegin(),
        tilesets.rend(),
        [gid](auto ts)
        {
            return gid >= ts->firstGid;
        }
    );

    int tileid = gid - tileset->firstGid;
    int x = (tileid % tileset->columns) * (tileset->tileWidth + tileset->spacing);
    int y = (tileid / tileset->columns) * (tileset->tileWidth + tileset->spacing);

    int textureRectWidth = tileset->tileWidth;
    int textureRectHeight = tileset->tileHeight;

    if (flipFlags & 2)
    {
        textureRectHeight *= -1;
        y += tileset->tileHeight;
    }

    if (flipFlags & 4)
    {
        textureRectWidth *= -1;
        x += tileset->tileWidth;
    }

    return std::tuple<sf::Texture&, sf::Rect<int>>(
        tileset->texture,
        sf::Rect<int>({x, y}, {textureRectWidth, textureRectHeight})
    );
}

void Map::loadTileset(rapidjson::Value &tileset)
{
    auto ts = new Tileset();

    ts->firstGid   = tileset["firstgid"].GetUint();
    ts->columns    = tileset["columns"].GetInt();
    ts->tileWidth  = tileset["tilewidth"].GetInt();
    ts->tileHeight = tileset["tileheight"].GetInt();
    ts->spacing    = tileset["spacing"].GetInt();

    if (ts->texture.loadFromFile(std::string("data/") + tileset["image"].GetString()))
    {
        tilesets.push_back(ts);
    }

    if (!tileset.HasMember("tiles"))
    {
        return;
    }

    for (rapidjson::Value &tile: tileset["tiles"].GetArray())
    {
        if (!tile.HasMember("animation"))
        {
            continue;
        }

        unsigned int animationId = ts->firstGid + tile["id"].GetInt();

        for (rapidjson::Value &animation: tile["animation"].GetArray())
        {
            unsigned int gid = ts->firstGid + animation["tileid"].GetUint();
            int duration = animation["duration"].GetInt();

            if (animations.find(animationId) == animations.end())
            {
                animations[animationId] = new std::vector<AnimationFrame*>();
            }

            animations[animationId]->push_back(new AnimationFrame(gid, duration));
        }
    }
}

unsigned int Map::getTileAt(int x, int y) const
{
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        return 1;
    }

    // map_F is the gameplay layer:
    // 0 = pellet/path, 1 = wall, 2 = gate, 3 = ghost house floor, 4 = power pellet, 5 = fruit.
    // Prefer it so map_B cannot create invisible walls.
    for (auto &obj : objects)
    {
        auto tileLayer = dynamic_cast<Layer*>(obj);

        if (!tileLayer)
        {
            continue;
        }

        if (tileLayer->name != "map_F")
        {
            continue;
        }

        if (x >= tileLayer->width || y >= tileLayer->height)
        {
            continue;
        }

        return tileLayer->tilemap[y * tileLayer->width + x];
    }

    // Fallback if the map does not have map_F.
    for (auto &obj : objects)
    {
        auto tileLayer = dynamic_cast<Layer*>(obj);

        if (!tileLayer)
        {
            continue;
        }

        if (x >= tileLayer->width || y >= tileLayer->height)
        {
            continue;
        }

        unsigned int tile = tileLayer->tilemap[y * tileLayer->width + x];

        if (tile != 0)
        {
            return tile;
        }
    }

    return 0;
}

bool Map::isWall(int x, int y)
{
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        return true;
    }

    return getTileAt(x, y) == 1;
}

void Map::loadTileLayer(rapidjson::Value &layer)
{
    auto tmp = new Layer(*this);

    tmp->id      = layer["id"].GetInt();
    tmp->name    = layer["name"].GetString();
    tmp->visible = layer["visible"].GetBool();
    tmp->width   = layer["width"].GetInt();
    tmp->height  = layer["height"].GetInt();
    tmp->tilemap = new unsigned int[tmp->width * tmp->height];

    const auto &tilemap = layer["data"].GetArray();

    if (static_cast<int>(tilemap.Size()) != tmp->width * tmp->height)
    {
        std::cout << "Warning: layer '" << tmp->name << "' has " << tilemap.Size()
                  << " tiles but expected " << tmp->width * tmp->height << std::endl;
    }

    for (size_t i = 0; i < tilemap.Size() && static_cast<int>(i) < tmp->width * tmp->height; i++)
    {
        tmp->tilemap[i] = tilemap[static_cast<rapidjson::SizeType>(i)].GetUint();
    }

    objects.push_back(tmp);
}

void Map::loadObjectLayer(rapidjson::Value &layer)
{
    if (!layer.HasMember("objects"))
    {
        return;
    }

    for (rapidjson::Value &object: layer["objects"].GetArray())
    {
        if (!object.HasMember("gid"))
        {
            continue;
        }

        auto sprite = new Sprite(*this);

        sprite->id     = object["id"].GetInt();
        sprite->gid    = object["gid"].GetUint();
        sprite->x      = object["x"].GetInt();
        sprite->y      = object["y"].GetInt();
        sprite->width  = object["width"].GetInt();
        sprite->height = object["height"].GetInt();
        sprite->y -= sprite->height;

        objects.push_back(sprite);
    }
}