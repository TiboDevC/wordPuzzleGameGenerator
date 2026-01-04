/*
 * Jeu de Mots Mélés sur E-Ink Display
 * Copyright (c) 2025
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// Wake-up interval: 20 minutes = 1200000 milliseconds
#define WAKE_INTERVAL_MS (20 * 60 * 1000)

// For initial demo, use shorter interval
#define DEMO_INTERVAL_MS (60 * 1000) // 1 minute for testing

// Simple 5x7 font for text display
static const uint8_t font5x7[][5] = {
	{0x00, 0x00, 0x00, 0x00, 0x00}, // Space
	{0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
	{0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
	{0x42, 0x61, 0x51, 0x49, 0x46}, // 2
	// ... (simplifié pour l'exemple)
};

static const struct device *display_dev;
static uint16_t display_width;
static uint16_t display_height;

void draw_pixel(uint16_t x, uint16_t y, bool color) {
	struct display_buffer_descriptor desc = {
		.buf_size = 1,
		.width = 1,
		.height = 1,
		.pitch = 1,
	};
	uint8_t pixel = color ? 0xFF : 0x00;
	display_write(display_dev, x, y, &desc, &pixel);
}

void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool color) {
	uint8_t pixel = color ? 0xFF : 0x00;
	
	// Allocate buffer for one line
	uint8_t *line_buf = (uint8_t*)k_malloc(w);
	if (!line_buf) {
		return;
	}
	
	memset(line_buf, pixel, w);
	
	struct display_buffer_descriptor desc = {
		.buf_size = w,
		.width = w,
		.height = 1,
		.pitch = w,
	};
	
	for (uint16_t row = y; row < y + h && row < display_height; row++) {
		display_write(display_dev, x, row, &desc, line_buf);
	}
	
	k_free(line_buf);
}

void draw_text_simple(uint16_t x, uint16_t y, const char* text, uint8_t scale) {
	// Simplified text drawing - just use display blanking for now
	// In a real implementation, we'd render the font properly
	LOG_INF("Text at (%d,%d): %s", x, y, text);
}

void display_hello_world() {
	LOG_INF("Updating display with Hello World...");

	// Clear the display (white)
	display_blanking_off(display_dev);
	
	// Draw border
	fill_rect(0, 0, display_width, 5, false);  // Top
	fill_rect(0, 0, 5, display_height, false); // Left
	fill_rect(display_width - 5, 0, 5, display_height, false); // Right
	fill_rect(0, display_height - 5, display_width, 5, false); // Bottom

	// For now, just log the text we would display
	// Full font rendering would be implemented later
	draw_text_simple(50, 30, "MOTS MELES", 4);
	draw_text_simple(50, 100, "Hello World!", 3);
	draw_text_simple(50, 180, "Ecran E-Ink 800x480", 2);
	draw_text_simple(50, 220, "Seeed XIAO nRF52840", 2);
	draw_text_simple(50, 300, "Reveil: 20 minutes", 2);
	
	static int iteration = 0;
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "Iteration: %d", iteration++);
	draw_text_simple(50, 350, buffer, 2);

	LOG_INF("Display updated successfully");
}

void enter_deep_sleep(uint32_t duration_ms) {
	LOG_INF("Preparing for deep sleep (%u ms)...", duration_ms);
	
	// Put display to sleep
	display_blanking_on(display_dev);
	
	// TODO: Configure RTC alarm for wake-up
	// For now, use a simple delay (not true deep sleep)
	k_msleep(duration_ms);
	
	LOG_INF("Waking up from sleep...");
}

int main() {
	LOG_INF("=== Jeu de Mots Meles - Starting ===");
	LOG_INF("Board: Seeed Studio XIAO nRF52840");
	LOG_INF("Display: 7.5\" E-Ink 800x480 GDEW075T7");
	
	// Get the display device
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready!");
		return -1;
	}
	
	// Get display capabilities
	struct display_capabilities caps;
	display_get_capabilities(display_dev, &caps);
	
	display_width = caps.x_resolution;
	display_height = caps.y_resolution;
	
	LOG_INF("Display initialized: %dx%d", display_width, display_height);
	LOG_INF("Pixel format: %d, Screen info: %d", 
			caps.current_pixel_format, caps.screen_info);
	
	// Main loop: wake up, update display, sleep
	while (true) {
		// Display Hello World on the e-ink screen
		display_hello_world();
		
		// Enter deep sleep for 20 minutes
		// Using demo interval (1 minute) for testing
		enter_deep_sleep(DEMO_INTERVAL_MS);
	}
	
	return 0;
}
