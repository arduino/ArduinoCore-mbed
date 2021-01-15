class CameraClass {
public:
	int begin(int horizontalResolution, int verticalResolution);
	int grab(uint8_t *buffer, uint32_t timeout=5000);
	void testPattern(bool walking);
	int skip_frames(uint8_t *buffer, uint32_t n_frames, uint32_t timeout=5000);
};
