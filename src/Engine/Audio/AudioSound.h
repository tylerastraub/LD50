#ifndef AUDIO_SOUND_H
#define AUDIO_SOUND_H

namespace AudioSound {
    enum AudioSound {
        NOVAL = -1,
        MOVE,
        SHOVE,
        PLACE_BOMB,
        BOMB_BLOW,
        REBUILD,
        ITEM_EARNED,
        SPAWN,
        FALL, // For when entities die
    };
};

#endif