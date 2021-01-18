enum {
    CAMERA_R160x120 = 0x00,   /* QQVGA Resolution */
    CAMERA_R320x240 = 0x01,   /* QVGA Resolution  */
    CAMERA_RMAX
};

class CameraClass {
    private:
        uint32_t resolution;
        bool     initialized;
    public:
        CameraClass(): initialized(false) {}
        int begin(uint32_t resolution = CAMERA_R320x240, uint32_t framerate = 30);
        int framerate(uint32_t framerate);
        int grab(uint8_t *buffer, uint32_t timeout=5000);
        int standby(bool enable);
        int testPattern(bool walking);
};
