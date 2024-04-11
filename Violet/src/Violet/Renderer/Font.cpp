#include "vlpch.h"
#include "Font.h"

#undef INFINITE
#include "msdf-atlas-gen.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"

#include "MSDFData.h"

namespace Violet {
	// 模板函数，用于创建和缓存 MSDF 纹理贴图
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
	{
		// 设置 MSDF 图像生成器的属性
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		// 创建 MSDF 纹理贴图
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(8);
		generator.generate(glyphs.data(), (int)glyphs.size());

		// 获取生成的图像数据
		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		// 创建纹理对象并设置数据
		TextureSpecification spec;
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = ImageFormat::RGB8;
		spec.GenerateMips = false;

		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * 3);
		return texture;
	}

	// 构造函数，用于加载字体文件并生成 MSDF 纹理贴图
	Font::Font(const std::filesystem::path& filepath)
		: m_Data(new MSDFData())
	{
		// 初始化 FreeType 库
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		VL_CORE_ASSERT(ft);

		std::string fileString = filepath.string();

		// TODO: msdfgen::loadFontData loads from memory buffer? 
		// 加载字体文件
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
		if (!font)
		{
			VL_CORE_ERROR("Failed to load font: {}", fileString);
			return;
		}

		// 定义字符集范围
		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		// 设置字符集范围
		// From imgui_draw.cpp
		static const CharsetRange charsetRanges[] =
		{
			{ 0x0020, 0x00FF }
		};

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t c = range.Begin; c <= range.End; c++)
				charset.add(c);
		}

		// 加载字符集到字形数据中
		double fontScale = 1.0;
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
		int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		VL_CORE_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());

		// 紧凑排版字形数据
		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;
		// atlasPacker.setDimensionsConstraint()
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);
		int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
		VL_CORE_ASSERT(remaining == 0);

		// 获取排版后的纹理宽度和高度，并创建 MSDF 纹理贴图
		int width, height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8
		// if MSDF || MTSDF

		uint64_t coloringSeed = 0;
		bool expensiveColoring = false;
		if (expensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, m_Data->Glyphs.size()).finish(THREAD_COUNT);
		}
		else {
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}


		m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

		// 释放字体资源
		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	// 析构函数，释放 MSDF 数据
	Font::~Font()
	{
		delete m_Data;
	}

	Ref<Font> Font::GetDefault()
	{
		static Ref<Font> DefaultFont;
		if (!DefaultFont)
			DefaultFont = CreateRef<Font>("assets/fonts/opensans/OpenSans-Regular.ttf");

		return DefaultFont;
	}
}