#ifndef PAC_MAN_LAYER_H
#define PAC_MAN_LAYER_H

#include "object.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <map>
#include <string>
#include <iostream>

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

    // Add these two members
    std::map<unsigned int, sf::Texture> textures;
    bool textureLoaded = false;

    void logic(float deltaTime) override {}

    void draw(sf::RenderWindow &window) override
    {
        if (!visible) return;

        if (!textureLoaded)
        {
            textures[1].loadFromFile("data/assets/walls/wallsquare.png");
            textures[2].loadFromFile("data/assets/walls/wallghosthouse.png");
            textureLoaded = true;
        }

        const float tileW = 500.f / 20.f;
        const float tileH = 500.f / 20.f;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = y * width + x;
                unsigned int tileValue = tilemap[index];

                if (tileValue == 0) continue;
                if (textures.find(tileValue) == textures.end()) continue;

                sf::Sprite sprite(textures[tileValue]);
                sprite.setPosition(sf::Vector2f(x * tileW, y * tileH));

                float scaleX = tileW / textures[tileValue].getSize().x;
                float scaleY = tileH / textures[tileValue].getSize().y;
                sprite.setScale(sf::Vector2f(scaleX, scaleY));

                window.draw(sprite);
            }
        }
    }

    virtual ~Layer() { delete[] tilemap; }

private:
    Map &map;
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

#endif //PAC_MAN_LAYER_H