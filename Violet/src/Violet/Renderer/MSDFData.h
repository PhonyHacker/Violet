#pragma once

#include <vector>

#undef INFINITE
#include "msdf-atlas-gen.h"

namespace Violet {
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;	// ���μ�������
		msdf_atlas::FontGeometry FontGeometry;			// ���弸������

	};
}