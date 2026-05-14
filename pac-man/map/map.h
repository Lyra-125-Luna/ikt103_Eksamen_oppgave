#ifndef MAP_H
#define MAP_H

#include "rapidjson/document.h"

#include <SFML/Graphics/Texture.hpp>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace sf
{
    class Sprite;
}

namespace Json
{
    class Value;
}

class Object;
class Layer;
class Sprite;

struct Tileset
{
    unsigned int firstGid;
    int columns;
    int tileWidth;
    int tileHeight;
    int spacing;

    sf::Texture texture;
};

struct AnimationFrame
{
    AnimationFrame(unsigned int gid, int duration) : gid(gid), duration(duration) {}

    unsigned int gid;
    int duration;
};

class Map
{
public:
    bool loadFromFile(const std::string &filename);

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    int getTileWidth() const { return tileWidth; }
    int getTileHeight() const { return tileHeight; }

    std::vector<AnimationFrame*> *getAnimation(unsigned int gid);

    std::tuple<const sf::Texture&, sf::Rect<int>> getSpriteTextureFromGid(unsigned int gid, int frame = 0);

    std::list<Object*> &GetObjects() { return objects; }

    unsigned int getTileAt(int x, int y) const;

    bool isWall(int x, int y);

protected:
    void loadTileset(rapidjson::Value &tileset);
    void loadTileLayer(rapidjson::Value &layer);
    void loadObjectLayer(rapidjson::Value &layer);

    int width = 0;
    int height = 0;
    int tileWidth = 25;
    int tileHeight = 25;

    std::list<Tileset*> tilesets;
    std::map<unsigned int, std::vector<AnimationFrame*>*> animations;

    std::list<Object*> objects;

    std::map<unsigned int, sf::Texture> textures;
    bool textureLoaded = false;
};

#endif