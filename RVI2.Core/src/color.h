/* FILE: color.h */

#pragma once

#include "rviTypes.h"

namespace rvi
{
	class Color
	{
	public:
		U8 R = 0;
		U8 G = 0;
		U8 B = 0;
		U8 A = 0;
		
		constexpr Color() noexcept { };
		
		Color(U8 r, U8 g, U8 b, U8 a) noexcept;
	};
}