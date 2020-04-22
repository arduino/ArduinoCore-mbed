#include "edid.h"
#include "video_modes.h"

struct envie_edid_mode envie_known_modes[NUM_KNOWN_MODES] = {
	[EDID_MODE_640x480_60Hz] = {
		.name = "640x480@75Hz", .pixel_clock = 29400, .refresh = 60,
		.hactive = 640, .hback_porch = 160, .hfront_porch = 16, .hsync_len = 96,
		.vactive = 480, .vback_porch = 45, .vfront_porch = 10, .vsync_len = 2,
	},
	[EDID_MODE_720x480_60Hz] = {
		.name = "720x480@60Hz", .pixel_clock = 27800, .refresh = 60,
		.hactive = 720, .hback_porch = 60, .hfront_porch = 16, .hsync_len = 62, 
		.vactive = 480, .vback_porch = 45, .vfront_porch = 9, .vsync_len = 6,
	},
	[EDID_MODE_800x600_59Hz] = {
		.name = "800x600@60Hz", .pixel_clock = 37800, .refresh = 60,
		.hactive = 800, .hback_porch = 104, .hfront_porch = 24, .hsync_len = 80, 
		.vactive = 600, .vback_porch = 17, .vfront_porch = 3, .vsync_len = 4,
	},
	[EDID_MODE_1024x768_60Hz] = {
		.name = "1024x768@60Hz", .pixel_clock = 57800, .refresh = 60,
		.hactive = 1024, .hback_porch = 80, .hfront_porch = 24, .hsync_len = 68, .hpol = 0,
		.vactive = 768, .vback_porch = 29, .vfront_porch = 3, .vsync_len = 6, .vpol = 0,
	},
	[EDID_MODE_1280x768_60Hz] = {
		.name = "1280x768@60Hz", .pixel_clock = 68300, .refresh = 60,
		.hactive = 1280, .hback_porch = 120, .hfront_porch = 32, .hsync_len = 20, 
		.vactive = 768, .vback_porch = 10, .vfront_porch = 45, .vsync_len = 12,
	},
	[EDID_MODE_1280x720_60Hz] = {
		.name = "1280x720@60Hz", .pixel_clock = 74300, .refresh = 60,
		.hactive = 1280, .hback_porch = 370, .hfront_porch = 110, .hsync_len = 40,
		.vactive = 720, .vback_porch = 30, .vfront_porch = 5, .vsync_len = 20,
	},
};
