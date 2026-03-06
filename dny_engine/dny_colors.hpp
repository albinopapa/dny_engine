#pragma once

#include "dny_type_traits.hpp"

#include <algorithm>
#include <cstdint>

namespace dny{
	class Color32{
	public:
		constexpr Color32()noexcept = default;
		constexpr explicit Color32( std::uint32_t value )noexcept
			:
			m_value( value ){}

		constexpr Color32( std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255ui8 )
			:
			m_value( ( alpha << 24 ) | ( red << 16 ) | ( green << 8 ) | ( blue ) ){}
		constexpr Color32( Color32 other, std::uint8_t alpha )noexcept
			:
			Color32( other.red(), other.green(), other.blue(), alpha ){}
		constexpr std::uint8_t alpha()const noexcept{
			return ( m_value & 0xFF000000 ) >> 24;
		}
		constexpr std::uint8_t red()const noexcept{
			return ( m_value & 0x00FF0000 ) >> 16;
		}
		constexpr std::uint8_t green()const noexcept{
			return ( m_value & 0x0000FF00 ) >> 8;
		}
		constexpr std::uint8_t blue()const noexcept{
			return m_value & 0x000000FF;
		}

		constexpr void alpha( std::uint8_t value_ )noexcept{
			m_value = ( ( m_value & 0x00FFFFFF ) | ( value_ << 24 ) );
		}
		constexpr void   red( std::uint8_t value_ )noexcept{
			m_value = ( ( m_value & 0xFF00FFFF ) | ( value_ << 16 ) );
		}
		constexpr void green( std::uint8_t value_ )noexcept{
			m_value = ( ( m_value & 0xFFFF00FF ) | ( value_ << 8 ) );
		}
		constexpr void  blue( std::uint8_t value_ )noexcept{
			m_value = ( ( m_value & 0xFFFFFF00 ) | value_ );
		}

		constexpr bool operator==( Color32 const& )const noexcept = default;

	private:
		std::uint32_t m_value = 0u;
	};

	class ColorF{
	public:
		constexpr ColorF()noexcept = default;
		constexpr explicit ColorF( Color32 other )noexcept
			:
			ColorF( other.red(), other.green(), other.blue(), other.alpha() ){}

		constexpr ColorF( std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255ui8 )
			:
			a( static_cast< float >( alpha ) * inv_max ),
			r( static_cast< float >( red )   * inv_max ),
			g( static_cast< float >( green ) * inv_max ),
			b( static_cast< float >( blue )  * inv_max ){}

		constexpr ColorF( float red, float green, float blue, float alpha = 1.f )
			: r( red ), g( green ), b( blue ), a( alpha ){}
		constexpr operator Color32()const noexcept{
			return Color32{
				static_cast< std::uint8_t >( r * 255.f ),
				static_cast< std::uint8_t >( g * 255.f ),
				static_cast< std::uint8_t >( b * 255.f ),
				static_cast< std::uint8_t >( a * 255.f )
			};
		}

		constexpr float alpha()const noexcept{
			return a;
		}
		constexpr float red()const noexcept{
			return r;
		}
		constexpr float green()const noexcept{
			return g;
		}
		constexpr float blue()const noexcept{
			return b;
		}

		constexpr void alpha( float value_ )noexcept{
			a = value_;
		}
		constexpr void   red( float value_ )noexcept{
			r = value_;
		}
		constexpr void green( float value_ )noexcept{
			g = value_;
		}
		constexpr void  blue( float value_ )noexcept{
			b = value_;
		}

		constexpr bool operator==( ColorF const& )const noexcept = default;

		constexpr ColorF& operator+=( ColorF const& other )noexcept{
			a += other.a;
			r += other.r;
			g += other.g;
			b += other.b;

			return *this;
		}
		constexpr ColorF& operator-=( ColorF const& other )noexcept{
			a -= other.a;
			r -= other.r;
			g -= other.g;
			b -= other.b;

			return *this;
		}
		constexpr ColorF& operator*=( float factor )noexcept{
			a *= factor;
			r *= factor;
			g *= factor;
			b *= factor;

			return *this;
		}
		constexpr ColorF& operator*=( ColorF factor )noexcept{
			a *= factor.a;
			r *= factor.r;
			g *= factor.g;
			b *= factor.b;

			return *this;
		}
		constexpr ColorF& operator/=( float other )noexcept{
			a /= other;
			r /= other;
			g /= other;
			b /= other;

			return *this;
		}

		constexpr ColorF& saturate()noexcept{
			a = std::clamp( a, 0.f, 1.f );
			r = std::clamp( r, 0.f, 1.f );
			g = std::clamp( g, 0.f, 1.f );
			b = std::clamp( b, 0.f, 1.f );

			return *this;
		}

	private:
		static constexpr float inv_max = 1.f / 255.f;
		float r = {};
		float g = {};
		float b = {};
		float a = 1.f;
	};

	constexpr auto operator+( ColorF const& lhs, ColorF const& rhs )noexcept{
		ColorF result = lhs;
		result += rhs;
		return result;
	}
	constexpr auto operator-( ColorF const& lhs, ColorF const& rhs )noexcept{
		ColorF result = lhs;
		result -= rhs;
		return result;
	}
	constexpr auto operator*( ColorF const& lhs, float rhs )noexcept{
		ColorF result = lhs;
		result *= rhs;
		return result;
	}
	constexpr auto operator*( float lhs, ColorF const& rhs )noexcept{
		ColorF result = rhs;
		result *= lhs;
		return result;
	}
	constexpr auto operator*( ColorF const& lhs, ColorF const& rhs )noexcept{
		ColorF result = lhs;
		result *= rhs;
		return result;
	}
	constexpr auto operator/( ColorF const& lhs, float rhs )noexcept{
		ColorF result = lhs;
		result /= rhs;
		return result;
	}


	namespace Colors{
		constexpr ColorF white = ColorF{ 1.0f, 1.0f, 1.0f };
		constexpr ColorF gray = ColorF{ 0.5f, 0.5f, 0.5f };
		constexpr ColorF black = ColorF{ 0.0f, 0.0f, 0.0f };
		constexpr ColorF red = ColorF{ 1.0f, 0.0f, 0.0f };
		constexpr ColorF green = ColorF{ 0.0f, 1.0f, 0.0f };
		constexpr ColorF blue = ColorF{ 0.0f, 0.0f, 1.0f };
		constexpr ColorF yellow = ColorF{ 1.0f, 1.0f, 0.0f };
		constexpr ColorF cyan = ColorF{ 0.0f, 1.0f, 1.0f };
		constexpr ColorF magenta = ColorF{ 1.0f, 0.0f, 1.0f };
	}
	
	template<> struct is_color<Color32> :std::true_type{};
	template<> struct is_color<ColorF> :std::true_type{};

}
