/*
 * Copyright (c) 2024-2025 silvio.vallorani.dev@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/libc-hooks.h>

// get the GPIO device
const struct gpio_dt_spec dbg_pin0 = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_pin0), gpios);
const struct gpio_dt_spec dbg_pin1 = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_pin1), gpios);

// Define the stack size and priority
#define THREAD_USERMODE_STACK_SIZE 2048
#define THREAD_USERMODE_PRIORITY 5

// Define the stack for the new thread
K_THREAD_STACK_DEFINE(thread_usermode_stack_area, THREAD_USERMODE_STACK_SIZE);

// Define the thread data structure
struct k_thread thread_usermode_data;
void thread_usermode(void *dummy1, void *dummy2, void *dummy3);
k_tid_t usermode_thread;

int main(void)
{
	__unused int ret;

	// Setup debug pins
	ret = gpio_is_ready_dt(&dbg_pin0);
	ret = gpio_pin_configure_dt(&dbg_pin0, GPIO_OUTPUT_ACTIVE);
	ret = gpio_is_ready_dt(&dbg_pin1);
	ret = gpio_pin_configure_dt(&dbg_pin1, GPIO_OUTPUT_ACTIVE);
	
	// Create the usermode thread in user mode
	usermode_thread = k_thread_create(&thread_usermode_data, thread_usermode_stack_area, THREAD_USERMODE_STACK_SIZE,
					thread_usermode, NULL, NULL, NULL,
					THREAD_USERMODE_PRIORITY, K_USER, K_NO_WAIT);

	// Grant access to the GPIO device to the user mode thread
	k_object_access_grant(dbg_pin1.port, usermode_thread);

	// Start the usermode thread
	k_thread_start(usermode_thread);

	while(1) {
		// Toggle debug pins just to see if the board is working
		ret = gpio_pin_toggle_dt(&dbg_pin0);
		k_msleep(1000);
	}
	
	return 0;
}

void thread_usermode(void *dummy1, void *dummy2, void *dummy3)
{
	__unused int ret;
	
	while(1) {
		ret = gpio_pin_toggle_dt(&dbg_pin1);
		k_msleep(1000);
	}
	
	return;
}

