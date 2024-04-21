#pragma once

#include "Violet/Core/Base.h"

namespace Violet {

	enum class FramebufferTextureFormat
	{
		None = 0,
		RGBA8,			// Color
		RED_INTEGER,	// Entity ID
		DEPTH24STENCIL8, // Depth/stencil
		Depth = DEPTH24STENCIL8 // Defaults
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		std::vector<FramebufferTextureFormat> Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};


}
