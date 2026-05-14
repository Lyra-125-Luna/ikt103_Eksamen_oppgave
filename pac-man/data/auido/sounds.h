//
// Created by luna on 5/11/26.
//

#ifndef PAC_MAN_SOUNDS_H
#define PAC_MAN_SOUNDS_H

#include <SFML/Audio.hpp>


class sounds_muscik
{

public:

    sounds_muscik() = default;
    bool getBuffer(void);
    void play();


private:

    sf::Music m_music;

    bool music = true;
    bool effects = true;

    std::vector <sf::Sound> aoudio;

    sf::Sound PixelPeeker;
    sf::Sound powerUp;
    sf::Sound pickUp;

};





#endif //PAC_MAN_SOUNDS_H