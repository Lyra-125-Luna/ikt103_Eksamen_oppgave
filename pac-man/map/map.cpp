#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include <SFML/Graphics/Sprite.hpp>

#include <memory>
#include <fstream>

#include "map.h"
#include "object/object.h"
#include "object/pac-man.h"
#include "object/enemy_gost_1.h"
#include "object/layer.h"

// Public functions

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

    // Get dimensions from first layer since root doesn't have them
    const auto &firstLayer = root["layers"][0];
    width     = firstLayer["width"].GetInt();
    height    = firstLayer["height"].GetInt();
    tileWidth  = 32; // hardcode or add to your JSON
    tileHeight = 32; // hardcode or add to your JSON

    // No tilesets in this format, skip loadTileset

    // Read in each layer
    for (auto &layer: root["layers"].GetArray())
    {
        std::string type = layer["type"].GetString();
        std::cout << "Loading layer: " << layer["name"].GetString() << std::endl;

        if (type == "tilelayer")
            loadTileLayer(layer);
        else
            loadObjectLayer(layer);
    }

    std::cout << "Map loaded successfully" << std::endl;
    return true;
}

std::vector<AnimationFrame*> *Map::getAnimation(unsigned int gid)
{
    auto animationIt = animations.find(gid);

    if (animationIt != animations.end())
        return animationIt->second;

    return nullptr;
}

std::tuple<const sf::Texture&, sf::Rect<int>> Map::getSpriteTextureFromGid(unsigned int gid, int frame)
{
    // Extract flip flags
    unsigned int flipFlags = gid >> 29;

    // Remove flip flags
    gid &= ~(0b111 << 29);

    // Look for an animation for this gid
    auto animationIt = animations.find(gid);

    if (animationIt != animations.end())
    {
        auto animation = *animationIt->second;

        if (frame >= 0 && frame < (int)animation.size())
            gid = animation[frame]->gid;
    }

    // Find the correct tileset for this gid
    auto tileset = *std::find_if(tilesets.rbegin(), tilesets.rend(), [gid](auto ts) { return gid >= ts->firstGid; });

    // Calculate x and y positions in the tileset
    int tileid = gid - tileset->firstGid;
    int x = (tileid % tileset->columns) * (tileset->tileWidth + tileset->spacing);
    int y = (tileid / tileset->columns) * (tileset->tileWidth + tileset->spacing);

    int textureRectWidth = tileset->tileWidth;
    int textureRectHeight = tileset->tileHeight;

    // Vertical flip
    if (flipFlags & 2)
    {
        textureRectHeight *= -1;
        y += tileset->tileHeight;
    }

    // Horizontal flip
    if (flipFlags & 4)
    {
        textureRectWidth *= -1;
        x += tileset->tileWidth;
    }

    return std::tuple<sf::Texture&, sf::Rect<int>>(tileset->texture, sf::Rect<int>({x, y}, {textureRectWidth, textureRectHeight}));
}

// Protected functions

void Map::loadTileset(rapidjson::Value &tileset)
{
    auto ts = new Tileset();

    ts->firstGid   = tileset["firstgid"].GetUint();
    ts->columns    = tileset["columns"].GetInt();
    ts->tileWidth  = tileset["tilewidth"].GetInt();
    ts->tileHeight = tileset["tileheight"].GetInt();
    ts->spacing    = tileset["spacing"].GetInt();

    if (ts->texture.loadFromFile(std::string("data/") + tileset["image"].GetString()))
        tilesets.push_back(ts);

    // Not all tilesets have tiles/animations
    if (!tileset.HasMember("tiles"))
        return;

    for (rapidjson::Value &tile: tileset["tiles"].GetArray())
    {
        // Not all tiles have animations
        if (!tile.HasMember("animation"))
            continue;

        unsigned int animationId = ts->firstGid + tile["id"].GetInt();

        for (rapidjson::Value &animation: tile["animation"].GetArray())
        {
            unsigned int gid = ts->firstGid + animation["tileid"].GetUint();
            int duration = animation["duration"].GetInt();

            if (animations.find(animationId) == animations.end())
                animations[animationId] = new std::vector<AnimationFrame*>();

            animations[animationId]->push_back(new AnimationFrame(gid, duration));
        }
    }
}

bool Map::isWall(int x, int y)
{
    for (auto &obj : objects)
    {
        auto tileLayer = dynamic_cast<Layer*>(obj);
        if (tileLayer && tileLayer->name == "map_B")
        {
            int index = y * tileLayer->width + x;
            return tileLayer->tilemap[index] == 1;
        }
    }
    return false;
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

    // Safety check
    if ((int)tilemap.Size() != tmp->width * tmp->height)
    {
        std::cout << "Warning: layer '" << tmp->name << "' has " << tilemap.Size()
                  << " tiles but expected " << tmp->width * tmp->height << std::endl;
    }

    for (size_t i = 0; i < tilemap.Size() && (int)i < tmp->width * tmp->height; i++)
        tmp->tilemap[i] = tilemap[(int)i].GetUint();

    objects.push_back(tmp);
}

void Map::loadObjectLayer(rapidjson::Value &layer)
{
    if (!layer.HasMember("objects"))
        return;

    for (rapidjson::Value &object: layer["objects"].GetArray())
    {
        // Some objects may not have a gid (non-tile objects)
        if (!object.HasMember("gid"))
            continue;

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