#pragma once
class ColorTransformation {
public:
	virtual ~ColorTransformation() = default;
	// r,g,b must be 0-255 anything else might result in undefined behaviour.
	virtual void apply(uint8_t&r, uint8_t&g, uint8_t&b) = 0;
};