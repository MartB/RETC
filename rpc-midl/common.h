#pragma once

typedef long            RZRESULT;           //!< Return result.
typedef unsigned int    RZDURATION;         //!< Milliseconds.
typedef void*           PRZPARAM;           //!< Context sensitive pointer.
typedef unsigned long           RZID;               //!< Generic data type for Identifier.
typedef unsigned long             RZCOLOR;            //!< Color data. 1st byte = Red; 2nd byte = Green; 3rd byte = Blue; 4th byte = Alpha (if applicable)

typedef enum EFFECT_TYPE_RETC
{
	KEYBOARD_NONE = 0,            //!< No effect.
	KEYBOARD_SOLID,
	KEYBOARD_DATA,
	KEYBOARD_DATA_AND_KEY,
	CHROMA_INVALID
} EFFECT_TYPE_RETC;
