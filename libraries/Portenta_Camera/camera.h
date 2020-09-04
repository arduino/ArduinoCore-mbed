class CameraClass {
public:
	int begin(int horizontalResolution, int verticalResolution);
	uint8_t* snapshot();
	int start(uint32_t timeout);
	uint8_t* grab(void);
	void testPattern(bool walking);
};