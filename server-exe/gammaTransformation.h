#pragma once
#include <array>
#include "colorTransformation.h"

class GammaTransformation : public ColorTransformation
{

public:
	GammaTransformation(Vec3D transformationValues) {
		LOG_I("Settings: r: {0} g: {1} b: {2}", transformationValues.x, transformationValues.y, transformationValues.z);
		// Create rgb gamma lookup table on startup => more performance
		for (int i = 0; i < 256; i++) {
			m_gammaArrayValues[RGB_CHANNELS::R][i] = (transformationValues.x != 0.0f) ? static_cast<BYTE>(pow(i / 256.0, 1.0 / transformationValues.x) * 256.0) : 0; //-V550
			m_gammaArrayValues[RGB_CHANNELS::G][i] = (transformationValues.y != 0.0f) ? static_cast<BYTE>(pow(i / 256.0, 1.0 / transformationValues.y) * 256.0) : 0; //-V550
			m_gammaArrayValues[RGB_CHANNELS::B][i] = (transformationValues.z != 0.0f) ? static_cast<BYTE>(pow(i / 256.0, 1.0 / transformationValues.z) * 256.0) : 0; //-V550
		}

	}

	void apply(int &r, int &g, int &b) {
		r = m_gammaArrayValues[RGB_CHANNELS::R][r];
		g = m_gammaArrayValues[RGB_CHANNELS::G][g];
		b = m_gammaArrayValues[RGB_CHANNELS::B][b];
	}

private:
	enum RGB_CHANNELS {
		R,
		G,
		B,
		MAX
	};

	typedef std::array<std::array<BYTE, 256>, RGB_CHANNELS::MAX> RGB_GAMMA_ARRAY;
	RGB_GAMMA_ARRAY m_gammaArrayValues;
};
