class CameraClass {
public:
	int begin(int horizontalResolution, int verticalResolution);
	uint8_t* snapshot();
	int start(void);
	uint8_t* grab(void);
	void testPattern(bool walking);
};