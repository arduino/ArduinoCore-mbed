#include "edid.h"
#include "video_modes.h"

struct envie_edid_mode envie_known_modes[NUM_KNOWN_MODES] = {
	[EDID_MODE_640x480_60Hz] = {
		.name = "640x480@75Hz", .pixel_clock = 26892, .refresh = 75,
		.hactive = 640, .hback_porch = 40, .hfront_porch = 8, .hsync_len = 32,
		.vactive = 480, .vback_porch = 6, .vfront_porch = 4, .vsync_len = 8,
	},
	[EDID_MODE_720x480_60Hz] = {
		.name = "720x480@60Hz", .pixel_clock = 26750, .refresh = 60,
		.hactive = 720, .hback_porch = 88, .hfront_porch = 24, .hsync_len = 64, .voffset = 0,
		.vactive = 480, .vback_porch = 7, .vfront_porch = 3, .vsync_len = 10,
	},
	[EDID_MODE_800x600_59Hz] = {
		.name = "800x600@60Hz", .pixel_clock = 37000, .refresh = 59,
		.hactive = 800, .hback_porch = 104, .hfront_porch = 24, .hsync_len = 80, .voffset = 1,
		.vactive = 600, .vback_porch = 17, .vfront_porch = 3, .vsync_len = 4,
	},
/*
	[EDID_MODE_1024x768_60Hz] = {
		.name = "1024x768@60Hz", .pixel_clock = 56000, .refresh = 60,
		.hactive = 1024, .hback_porch = 80, .hfront_porch = 48, .hsync_len = 32, .voffset = 1,
		.vactive = 768, .vback_porch = 15, .vfront_porch = 3, .vsync_len = 4,
	},
	[EDID_MODE_1280x768_60Hz] = {
		.name = "1280x768@60Hz", .pixel_clock = 68250, .refresh = 60,
		.hactive = 1280, .hback_porch = 120, .hfront_porch = 32, .hsync_len = 20, .voffset = 1,
		.vactive = 768, .vback_porch = 10, .vfront_porch = 45, .vsync_len = 12,
	},
	[EDID_MODE_1280x720_60Hz] = {
		.name = "1280x720@60Hz", .pixel_clock = 74250, .refresh = 60,
		.hactive = 1280, .hback_porch = 370, .hfront_porch = 110, .hsync_len = 40,
		.vactive = 720, .vback_porch = 30, .vfront_porch = 5, .vsync_len = 20,
	},
*/
};
