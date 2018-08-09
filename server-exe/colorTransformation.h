#pragma once
class ColorTransformation {
public:
	// r,g,b must be 0-255 anything else might result in undefined behaviour.
	virtual void apply(int &r, int &g, int &b) = 0;
};