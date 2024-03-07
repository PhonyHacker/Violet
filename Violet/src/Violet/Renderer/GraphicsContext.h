#pragma once

namespace Violet {
	class VIOLET_API   GraphicsContext {
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}