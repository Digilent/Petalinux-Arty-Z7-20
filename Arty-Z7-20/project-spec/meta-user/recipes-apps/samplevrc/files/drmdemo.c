#include <libvrc.h>

int main() {

    struct drm_mode_modeinfo test_mode = {DRM_MODE("1600x900", DRM_MODE_TYPE_DRIVER, 108000, 1600, 1624, \
        1704, 1800, 0, 900, 901, 904, 1000, 0, \
        DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)};
    drm_cntrl * test;
    test = drmControlInit("/dev/dri/card0", test_mode);

    uint32_t currentSize = test->create_dumb[test->current_fb]->size;

    uint8_t * mem = (uint8_t *)test->fbMem[0];
    for(int i = 0; i < currentSize; i++) {
        if (i % 3 == 1 || i % 3 == 0) {
            *(mem + i) = 0xFF;
        } else {
            *(mem + i) = 0x00;
        }
    }

    uint8_t * mem2 = (uint8_t *)test->fbMem[1];
    for(int i = 0; i < currentSize; i += 3) {
        *(mem2 + i) = i;
        *(mem2 + i + 1) = i;
        *(mem2 + i + 2) = i;
    }

    char c;
    while(1) {
        c = fgetc(stdin);
        if(c == 'b') {
            mapFBtoCRTC(test, 1);
        } else if (c == 'a') {
            mapFBtoCRTC(test, 0);
        } else if( c == 'x') {
            break;
        }
    }

    // struct drm_mode_modeinfo test2_mode = {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 78750, 1024, 1040, \
    //     1136, 1312, 0, 768, 769, 772, 800, 0, \
    //     DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)};

    // setNewResolution(test, test2_mode);

    // currentSize = test->create_dumb[test->current_fb]->size;
    // mem = (uint8_t *)test->fbMem[0];
    // mem2 = (uint8_t *)test->fbMem[1];
    // for(int i = 0; i < currentSize; i++) {
    //     if (i % 3 == 1 || i % 3 == 2) {
    //         *(mem + i) = 0xFF;
    //     } else {
    //         *(mem + i) = 0x00;
    //     }
    // }

    // for(int i = 0; i < currentSize; i += 3) {
    //     if(i % 3 == 2) {
    //         *(mem2 + i) = 0xFF;
    //     } else {
    //         *(mem2 + i) = 0;
    //     }
    // }

    // while(1) {
    //     c = fgetc(stdin);
    //     if(c == 'b') {
    //         mapFBtoCRTC(test, 1);
    //     } else if (c == 'a') {
    //         mapFBtoCRTC(test, 0);
    //     } else if( c == 'x') {
    //         break;
    //     }
    // }


    drmControlClose(test);
    return SUCCESS;
}
