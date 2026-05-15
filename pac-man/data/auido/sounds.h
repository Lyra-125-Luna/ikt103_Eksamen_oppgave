#ifndef PAC_MAN_SOUNDS_H
#define PAC_MAN_SOUNDS_H

#include <SFML/Audio.hpp>
#include <optional>

class sounds_muscik
{
public:
    bool getBuffer();
    void playMusic();
    void stopMusic();
    void playPickUp();
    void playPowerUp(float duration);                // plays power-up and pauses music
    void updatePowerUpState(float deltaTime);       // call every game tick to resume music when done

private:
    sf::SoundBuffer bufferMusic;
    sf::SoundBuffer bufferPowerUp;
    sf::SoundBuffer bufferPickUp;

    std::optional<sf::Sound> PixelPeeker;
    std::optional<sf::Sound> powerUp;
    std::optional<sf::Sound> pickUp;

    bool powerUpPlaying = false;
    float powerUpTimer = 0.f;
};

#endif