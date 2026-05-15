#include "sounds.h"

void SoundManager::load()
{
    if (musicBuffer.loadFromFile("data/audio/Pixel-Peeker-Polka-faster(chosic.com).mp3"))
    {
        music.emplace(musicBuffer);
    }

    if (powerUpBuffer.loadFromFile("data/audio/power_up.wav"))
    {
        powerUp.emplace(powerUpBuffer);
    }

    if (pickUpBuffer.loadFromFile("data/audio/pickUp.wav"))
    {
        pickUp.emplace(pickUpBuffer);
    }
}

void SoundManager::playMusic()
{
    if (music)
    {
        music->play();
    }
}

void SoundManager::playPickUp()
{
    if (pickUp)
    {
        pickUp->play();
    }
}

void SoundManager::playPowerUp(float duration)
{
    if (!powerUp)
    {
        return;
    }

    if (music)
    {
        music->stop();
    }

    powerUp->play();
    powerUpPlaying = true;
    powerUpTimer = duration;
}

void SoundManager::stopMusic()
{
    if (music)
    {
        music->stop();
    }
}

void SoundManager::updatePowerUpState(float deltaTime)
{
    if (!powerUpPlaying)
    {
        return;
    }

    powerUpTimer -= deltaTime;

    if (powerUpTimer <= 0.f)
    {
        powerUpTimer = 0.f;
        powerUpPlaying = false;

        if (music)
        {
            music->play();
        }
    }
}
