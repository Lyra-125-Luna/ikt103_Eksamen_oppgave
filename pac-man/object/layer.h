#ifndef PAC_MAN_LAYER_H
#define PAC_MAN_LAYER_H

#include "object.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <map>
#include <string>

class Map;

class Layer : public Object
{
public:
    Layer(Map &map) : map(map), id(0), visible(true), width(0), height(0) {}

    int id;
    std::string name;
    bool visible;
    int width;
    int height;
    unsigned int *tilemap = nullptr;

    std::map<std::string, sf::Texture> textures;
    bool textureLoaded = false;

    void logic(float deltaTime) override {}

    void draw(sf::RenderWindow &window) override
    {
        if (!visible)
        {
            return;
        }
        if (name != "map_F")
        {
            return;
        }

        if (!textureLoaded)
        {
            loadTextures();
            textureLoaded = true;
        }

        const float tileW = 25.f;
        const float tileH = 25.f;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = y * width + x;
                unsigned int tileValue = tilemap[index];

                if (tileValue == 0)
                {
                    continue;
                }

                if (tileValue == 1)
                {
                    drawAutoWall(window, x, y, tileW, tileH);
                    continue;
                }

                if (tileValue == 2)
                {
                    drawTile(window, "ghosthouse", x, y, 0.f, tileW, tileH);
                    continue;
                }

                if (tileValue == 3)
                {
                    continue;
                }

                if (tileValue == 4)
                {
                    continue;
                }

                if (tileValue == 5)
                {
                    continue;
                }
            }
        }
    }

    virtual ~Layer()
    {
        delete[] tilemap;
    }

private:
    Map &map;

    void loadTextures()
    {
        loadTexture("center", "data/assets/walls/wallcenter.png");
        loadTexture("corner", "data/assets/walls/wallcorner.png");
        loadTexture("edge", "data/assets/walls/walledge.png");
        loadTexture("ghosthouse", "data/assets/walls/wallghosthouse.png");
        loadTexture("l", "data/assets/walls/walll.png");
        loadTexture("shaft", "data/assets/walls/wallshaft.png");
        loadTexture("square", "data/assets/walls/wallsquare.png");
        loadTexture("stub", "data/assets/walls/wallstub.png");
        loadTexture("t", "data/assets/walls/wallt.png");
    }

    void loadTexture(const std::string& key, const std::string& path)
    {
        textures[key].loadFromFile(path);
    }

    bool isWallValue(unsigned int value) const
    {
        return value == 1;
    }

    bool isWallAt(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
        {
            return false;
        }

        unsigned int value = tilemap[y * width + x];
        return isWallValue(value);
    }

    int getWallMask(int x, int y) const
    {
        int mask = 0;

        if (isWallAt(x, y - 1)) mask |= 1;
        if (isWallAt(x + 1, y)) mask |= 2;
        if (isWallAt(x, y + 1)) mask |= 4;
        if (isWallAt(x - 1, y)) mask |= 8;

        return mask;
    }

    void drawAutoWall(sf::RenderWindow& window, int x, int y, float tileW, float tileH)
    {
        int mask = getWallMask(x, y);

        std::string textureKey = "square";
        float rotation = 0.f;

        switch (mask)
        {
            case 0:
                textureKey = "square";
                rotation = 0.f;
                break;

            case 1:
                textureKey = "stub";
                rotation = 180.f;
                break;

            case 2:
                textureKey = "stub";
                rotation = 270.f;
                break;

            case 4:
                textureKey = "stub";
                rotation = 0.f;
                break;

            case 8:
                textureKey = "stub";
                rotation = 90.f;
                break;

            case 5:
                textureKey = "shaft";
                rotation = 0.f;
                break;

            case 10:
                textureKey = "shaft";
                rotation = 90.f;
                break;

            case 6:
                textureKey = "corner";
                rotation = 0.f;
                break;

            case 12:
                textureKey = "corner";
                rotation = 90.f;
                break;

            case 9:
                textureKey = "corner";
                rotation = 180.f;
                break;

            case 3:
                textureKey = "corner";
                rotation = 270.f;
                break;

            case 7:
                textureKey = "t";
                rotation = 0.f;
                break;

            case 14:
                textureKey = "t";
                rotation = 90.f;
                break;

            case 13:
                textureKey = "t";
                rotation = 180.f;
                break;

            case 11:
                textureKey = "t";
                rotation = 270.f;
                break;

            case 15:
                textureKey = "center";
                rotation = 0.f;
                break;

            default:
                textureKey = "square";
                rotation = 0.f;
                break;
        }

        drawTile(window, textureKey, x, y, rotation, tileW, tileH);
    }

    void drawTile(
        sf::RenderWindow& window,
        const std::string& textureKey,
        int x,
        int y,
        float rotation,
        float tileW,
        float tileH
    )
    {
        auto it = textures.find(textureKey);

        if (it == textures.end())
        {
            return;
        }

        sf::Texture& texture = it->second;
        sf::Sprite sprite(texture);

        sf::Vector2u textureSize = texture.getSize();

        if (textureSize.x == 0 || textureSize.y == 0)
        {
            return;
        }

        sprite.setOrigin({
            static_cast<float>(textureSize.x) / 2.f,
            static_cast<float>(textureSize.y) / 2.f
        });

        sprite.setPosition({
            x * tileW + tileW / 2.f,
            y * tileH + tileH / 2.f
        });

        sprite.setScale({
            tileW / static_cast<float>(textureSize.x),
            tileH / static_cast<float>(textureSize.y)
        });

        sprite.setRotation(sf::degrees(rotation));

        window.draw(sprite);
    }
};

class Sprite : public Object
{
public:
    Sprite(Map &map) : map(map), id(0), gid(0), x(0), y(0), width(0), height(0) {}

    int id;
    unsigned int gid;
    int x;
    int y;
    int width;
    int height;

    void logic(float deltaTime) override {}
    void draw(sf::RenderWindow &window) override {}

    virtual ~Sprite() {}

private:
    Map &map;
};

#endif