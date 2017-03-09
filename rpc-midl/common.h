#pragma once

typedef enum RETCDeviceType
{
	KEYBOARD,
	MOUSE,
	HEADSET,
	MOUSEPAD,
	KEYPAD,
	MAX
} RETCDeviceType;


typedef struct tagSupportArray {
	BOOL m_keyboard;
	BOOL m_mouse;
	BOOL m_headset;
	BOOL m_mousepad;
	BOOL m_keypad;
} supportArray_t;