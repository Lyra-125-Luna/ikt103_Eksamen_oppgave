
#include "sounds.h"

#include "game.h"

game game;


#include <iostream>
#include <SFML/Audio.hpp>
#include <vector>

bool m = false;



bool sounds_muscik::getBuffer()
{
    sf::SoundBuffer bufferMusic;
    if (!bufferMusic.loadFromFile("data/auido/Pixel-Peeker-Polka-faster(chosic.com).mp3"))
    {
        std::cout << "No musick error" << std::endl;
    }

    sf::SoundBuffer bufferPowerUp;
    if (!bufferPowerUp.loadFromFile("data/auido/power_up.wav"))
    {
        std::cout << "No powerUp error" << std::endl;
    }

    sf::SoundBuffer pickUp_S;
    if (!pickUp_S.loadFromFile("data/auido/pickUp.wav"))
    {
        std::cout << "No pickUp error" << std::endl;
    }



    PixelPeeker.setBuffer(bufferMusic);
    powerUp.setBuffer(bufferPowerUp);
    pickUp.setBuffer(pickUp_S);

    aoudio.push_back(PixelPeeker);
    aoudio.push_back(powerUp);
    aoudio.push_back(PixelPeeker);

    m = true;
    bool loadSounds(m);


    return true;
}
