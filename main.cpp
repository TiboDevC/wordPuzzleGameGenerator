/*
 * Jeu de Mots Mélés sur E-Ink Display
 * Copyright (c) 2025
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// LED colors enumeration
enum class LedColor : uint8_t {
	RED = 0,
	GREEN = 1,
	BLUE = 2,
	NUM_COLORS = 3
};

// Timing constants
namespace timing {
	constexpr uint32_t SECONDS_PER_MINUTE = 60;
	constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
	constexpr uint32_t LED_CYCLE_INTERVAL_MS = MILLISECONDS_PER_SECOND;
	constexpr uint32_t DISPLAY_UPDATE_INTERVAL_SECONDS = 60;
	constexpr uint32_t WAKE_INTERVAL_MINUTES = 20;
	constexpr uint32_t WAKE_INTERVAL_MS = WAKE_INTERVAL_MINUTES * SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
}

// Display constants
namespace display {
	constexpr uint16_t BORDER_THICKNESS = 5;
}

// LED GPIO specs from device tree
static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

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

static LedColor current_led_color = LedColor::RED;

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

void set_led_color(LedColor color) {
	constexpr int LED_OFF = 0;
	constexpr int LED_ON = 1;
	
	// Turn off all LEDs first
	gpio_pin_set_dt(&led_red, LED_OFF);
	gpio_pin_set_dt(&led_green, LED_OFF);
	gpio_pin_set_dt(&led_blue, LED_OFF);
	
	// Turn on selected color
	switch (color) {
		case LedColor::RED:
			gpio_pin_set_dt(&led_red, LED_ON);
			LOG_INF("LED: RED");
			break;
		case LedColor::GREEN:
			gpio_pin_set_dt(&led_green, LED_ON);
			LOG_INF("LED: GREEN");
			break;
		case LedColor::BLUE:
			gpio_pin_set_dt(&led_blue, LED_ON);
			LOG_INF("LED: BLUE");
			break;
		case LedColor::NUM_COLORS:
			// Should never happen
			break;
	}
}

LedColor get_next_led_color(LedColor current) {
	return static_cast<LedColor>((static_cast<uint8_t>(current) + 1) % static_cast<uint8_t>(LedColor::NUM_COLORS));
}

void display_hello_world() {
	LOG_INF("Updating display with Hello World...");
	
	constexpr bool BLACK = false;

	// Clear the display (white)
	display_blanking_off(display_dev);
	
	// Draw border
	const uint16_t border = display::BORDER_THICKNESS;
	fill_rect(0, 0, display_width, border, BLACK);  // Top
	fill_rect(0, 0, border, display_height, BLACK); // Left
	fill_rect(display_width - border, 0, border, display_height, BLACK); // Right
	fill_rect(0, display_height - border, display_width, border, BLACK); // Bottom

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
	
	// Initialize LEDs
	if (!gpio_is_ready_dt(&led_red) || !gpio_is_ready_dt(&led_green) || !gpio_is_ready_dt(&led_blue)) {
		LOG_ERR("LED GPIO devices not ready!");
		return -1;
	}
	
	gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_blue, GPIO_OUTPUT_INACTIVE);
	
	// Set initial LED color
	set_led_color(current_led_color);
	
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
	
	// Main loop: wake up, update display, cycle LED
	while (true) {
		// Display Hello World on the e-ink screen
		display_hello_world();
		
		// Cycle LED color every second to show activity
		// RED -> GREEN -> BLUE -> repeat
		for (uint32_t i = 0; i < timing::DISPLAY_UPDATE_INTERVAL_SECONDS; i++) {
			current_led_color = get_next_led_color(current_led_color);
			set_led_color(current_led_color);
			k_msleep(timing::LED_CYCLE_INTERVAL_MS);
		}
		
		LOG_INF("Display update cycle complete");
	}
	
	return 0;
}
