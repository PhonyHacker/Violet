#pragma once

#include <vector>

#undef INFINITE
#include "msdf-atlas-gen.h"

namespace Violet {
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;	// 字形几何数据
		msdf_atlas::FontGeometry FontGeometry;			// 字体几何数据

	};
}