/**
 * @fileoverview This file contains the configuration for the camera.
 * @author Sebastian Romero
 */

/**
 * The available camera (color) modes.
 * The Arduino sketch uses the same values to communicate which mode should be used.
 **/
const CAMERA_MODES = {
    0: "GRAYSCALE",
    1: "BAYER",
    2: "RGB565"
};

/**
 * The available camera resolutions.
 * The Arduino sketch uses the same values to communicate which resolution should be used.
 */
const CAMERA_RESOLUTIONS = {
    0: {
        "name": "QQVGA",
        "width": 160,
        "height": 120
    },
    1: {
        "name": "QVGA",
        "width": 320,
        "height": 240
    },
    2: {
        "name": "320x320",
        "width": 320,
        "height": 320
    },
    3: {
        "name": "VGA",
        "width": 640,
        "height": 480
    },
    5: {
        "name": "SVGA",
        "width": 800,
        "height": 600
    },
    6: {
        "name": "UXGA",
        "width": 1600,
        "height": 1200
    }
};