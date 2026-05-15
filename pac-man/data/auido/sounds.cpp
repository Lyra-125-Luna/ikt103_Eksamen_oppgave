#include "sounds.h"
#include <iostream>

bool sounds_muscik::getBuffer()
{
    if (!bufferMusic.loadFromFile("data/auido/Pixel-Peeker-Polka-faster(chosic.com).mp3"))
    {
        std::cout << "No music error" << std::endl;
        return false;
    }
    if (!bufferPowerUp.loadFromFile("data/auido/power_up.wav"))
    {
        std::cout << "No powerUp error" << std::endl;
        return false;
    }
    if (!bufferPickUp.loadFromFile("data/auido/pickUp.wav"))
    {
        std::cout << "No pickUp error" << std::endl;
        return false;
    }

    PixelPeeker.emplace(bufferMusic);
    powerUp.emplace(bufferPowerUp);
    pickUp.emplace(bufferPickUp);

    return true;
}

void sounds_muscik::playMusic()
{
    if (PixelPeeker) PixelPeeker->play();
}

void sounds_muscik::playPickUp()
{
    if (pickUp) pickUp->play();
}

void sounds_muscik::playPowerUp(float duration)
{
    if (!powerUp || !PixelPeeker) return;

    PixelPeeker->stop();
    powerUp->play();
    powerUpPlaying = true;
    powerUpTimer = duration;        // store the full power mode duration
}

void sounds_muscik::stopMusic()
{
    if (PixelPeeker) PixelPeeker->stop();
}

void sounds_muscik::updatePowerUpState(float deltaTime)
{
    if (!powerUpPlaying) return;

    powerUpTimer -= deltaTime;

    if (powerUpTimer <= 0.f)
    {
        powerUpTimer = 0.f;
        powerUpPlaying = false;

        if (PixelPeeker) PixelPeeker->play();
    }
}