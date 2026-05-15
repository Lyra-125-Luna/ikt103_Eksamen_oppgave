#ifndef PAC_MAN_SOUNDS_H
#define PAC_MAN_SOUNDS_H

#include <SFML/Audio.hpp>
#include <optional>

class SoundManager
{
public:
    void load();
    void playMusic();
    void stopMusic();
    void playPickUp();
    void playPowerUp(float duration);
    void updatePowerUpState(float deltaTime);

private:
    sf::SoundBuffer musicBuffer;
    sf::SoundBuffer powerUpBuffer;
    sf::SoundBuffer pickUpBuffer;

    std::optional<sf::Sound> music;
    std::optional<sf::Sound> powerUp;
    std::optional<sf::Sound> pickUp;

    bool powerUpPlaying = false;
    float powerUpTimer = 0.f;
};

#endif
