
#include "sounds.h"

#include <iostream>
#include <SFML/Audio.hpp>

bool sounds::sounds_active()
{
    sf::SoundBuffer music("Pixel-Peeker-Polka-faster(chosic.com).mp3");
    if (!music.loadFromFile("Pixel-Peeker-Polka-faster(chosic.com).mp3"))
    {
        std::cerr << "Could not load music." << std::endl;
        return false;
    }

    sf::SoundBuffer power("power_up.wav");
    if (!power.loadFromFile("Power-up.wav"))
    {
        std::cerr << "Could not load power." << std::endl;
        return false;
    }

    sf::SoundBuffer revive("revive.wav");
    if (!revive.loadFromFile("revive.wav"))
    {
        std::cerr << "Could not load revive." << std::endl;
        return false;
    }


    return true;
}

bool sounds::sounds_deactive()
{
    return false;

}