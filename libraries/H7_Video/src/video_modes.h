struct envie_edid_mode {
	const char *name;
	unsigned int pixel_clock;
	unsigned int refresh;
	unsigned int hactive;
	unsigned int hback_porch;
	unsigned int hfront_porch;
	unsigned int hsync_len;
	unsigned int vactive;
	unsigned int vsync_len;
	unsigned int vback_porch;
	unsigned int vfront_porch;
	unsigned int voffset;
	unsigned int hpol : 1;
	unsigned int vpol : 1;
};