#pragma once

namespace gfx
{
	enum BUFFER_USAGE
	{
		STATIC,
		DYNAMIC
	};

	enum GFX_Primitive
	{
		POINTS,
		LINES,
		LINE_STRIP,
		TRIANGLES
	};

	enum Wrapping_Mode
	{
		REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRRORED_REPEAT
	};

	enum Filtering_Mode
	{
		NEAREST,
		LINEAR,
		NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR,
	};

	enum FrameBuffer_Mode
	{
		RenderBuffer,
		DepthBuffer,
		DepthCubeMap,
	};

} // namespace gfx