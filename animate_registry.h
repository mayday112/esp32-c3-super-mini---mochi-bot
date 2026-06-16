#ifndef __ANIMATION_REGISTRY_
#define __ANIMATION_REGISTRY_

#include <Arduino.h>

#define SCREEN_SIZE_WIDTH 128
#define SCREEN_SIZE_HEIGHT 64

typedef struct {
    const uint8_t frame_count;
    const uint16_t width;
    const uint16_t height;
    const uint16_t* delays;
    const unsigned char* const* frames;
} Animate;

#include "angry.h"
#include "cute.h"
#include "gigle.h"
#include "glance.h"
#include "grumpy.h"
#include "happy.h"
#include "intro.h"
#include "sad.h"
#include "standby.h"
#include "uwu.h"


static const Animate playlist[] = {
    intro,// intro
    standbyFaceGif,//idle
    glanceFaceGif,//one tap
    angryFaceGif,//double tap [random]
    sadFaceGif,//double tap [random]
    grumpyFaceGif,//double tap [random]
    giggleFaceGif,//double tap [random]
    happyFaceGif,//double tap [random]
    cuteFaceGif,//long press [random]
    uwuFaceGif//long press > 3s
};

const uint8_t TOTAL_VIDEOS =  sizeof(playlist) / sizeof(playlist[0]);

#endif // __ASSETS_H__
