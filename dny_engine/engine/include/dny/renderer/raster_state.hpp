#pragma once

#include "graphics/colors.hpp"

#include <cstdint>
#include <concepts>

namespace dny{
	// Enums for raster state configuration
	enum class blend_factor : std::uint8_t{
		zero,
		one,
		src_color,
		inv_src_color,
		dst_color,
		inv_dst_color,
		src_alpha,
		inv_src_alpha,
		dst_alpha,
		inv_dst_alpha
	};

	enum class blend_op : std::uint8_t{
		add,
		subtract,
		rev_subtract,
		min,
		max
	};

	enum class depth_func : std::uint8_t{
		never,
		less,
		less_equal,
		equal,
		greater,
		greater_equal,
		not_equal,
		always
	};

	enum class cull_mode : std::uint8_t{
		none,
		front,
		back
	};

	enum class front_face : std::uint8_t{
		cw,
		ccw
	};

	// Helper functions for blend operations
	namespace detail{
		constexpr float eval_blend_factor(
			blend_factor factor,
			ColorF const& src,
			ColorF const& dst ) noexcept{
			switch( factor ){
				case blend_factor::zero: return 0.0f;
				case blend_factor::one: return 1.0f;
				case blend_factor::src_color: return 1.0f; // Placeholder for per-channel
				case blend_factor::inv_src_color: return 0.0f; // Placeholder
				case blend_factor::dst_color: return 1.0f; // Placeholder
				case blend_factor::inv_dst_color: return 0.0f; // Placeholder
				case blend_factor::src_alpha: return src.alpha();
				case blend_factor::inv_src_alpha: return 1.0f - src.alpha();
				case blend_factor::dst_alpha: return dst.alpha();
				case blend_factor::inv_dst_alpha: return 1.0f - dst.alpha();
				default: return 1.0f;
			}
		}

		constexpr ColorF eval_blend_factor_color(
			blend_factor factor,
			ColorF const& src,
			ColorF const& dst ) noexcept{
			switch( factor ){
				case blend_factor::zero: return ColorF{ 0.0f, 0.0f, 0.0f, 0.0f };
				case blend_factor::one: return ColorF{ 1.0f, 1.0f, 1.0f, 1.0f };
				case blend_factor::src_color: return src;
				case blend_factor::inv_src_color: return ColorF{
					1.0f - src.red(),
					1.0f - src.green(),
					1.0f - src.blue(),
					1.0f - src.alpha()
				};
				case blend_factor::dst_color: return dst;
				case blend_factor::inv_dst_color: return ColorF{
					1.0f - dst.red(),
					1.0f - dst.green(),
					1.0f - dst.blue(),
					1.0f - dst.alpha()
				};
				case blend_factor::src_alpha:
				{
					const float a = src.alpha();
					return ColorF{ a, a, a, a };
				}
				case blend_factor::inv_src_alpha:
				{
					const float inv = 1.0f - src.alpha();
					return ColorF{ inv, inv, inv, inv };
				}
				case blend_factor::dst_alpha:
				{
					const float a = dst.alpha();
					return ColorF{ a, a, a, a };
				}
				case blend_factor::inv_dst_alpha:
				{
					const float inv = 1.0f - dst.alpha();
					return ColorF{ inv, inv, inv, inv };
				}
				default: return ColorF{ 1.0f, 1.0f, 1.0f, 1.0f };
			}
		}

		constexpr float apply_blend_op( blend_op op, float src_val, float dst_val ) noexcept{
			switch( op ){
				case blend_op::add: return src_val + dst_val;
				case blend_op::subtract: return src_val - dst_val;
				case blend_op::rev_subtract: return dst_val - src_val;
				case blend_op::min: return src_val < dst_val ? src_val : dst_val;
				case blend_op::max: return src_val > dst_val ? src_val : dst_val;
				default: return src_val + dst_val;
			}
		}

		constexpr bool depth_test( depth_func func, float src_depth, float dst_depth ) noexcept{
			switch( func ){
				case depth_func::never: return false;
				case depth_func::less: return src_depth < dst_depth;
				case depth_func::less_equal: return src_depth <= dst_depth;
				case depth_func::equal: return src_depth == dst_depth;
				case depth_func::greater: return src_depth > dst_depth;
				case depth_func::greater_equal: return src_depth >= dst_depth;
				case depth_func::not_equal: return src_depth != dst_depth;
				case depth_func::always: return true;
				default: return src_depth < dst_depth;
			}
		}
	}

	// Blend function using policy
	template<typename BlendPolicy>
	constexpr ColorF blend( ColorF const& src, ColorF const& dst ) noexcept{
		if constexpr( !BlendPolicy::blend_enabled ){
			return src;
		}
		else{
			const auto src_factor_color = detail::eval_blend_factor_color(
				BlendPolicy::src_color_factor, src, dst );
			const auto dst_factor_color = detail::eval_blend_factor_color(
				BlendPolicy::dst_color_factor, src, dst );
			const auto src_factor_alpha = detail::eval_blend_factor(
				BlendPolicy::src_alpha_factor, src, dst );
			const auto dst_factor_alpha = detail::eval_blend_factor(
				BlendPolicy::dst_alpha_factor, src, dst );

			ColorF result;
			result = ColorF{
				detail::apply_blend_op(
					BlendPolicy::color_operation,
					src.red() * src_factor_color.red(),
					dst.red() * dst_factor_color.red() ),
					detail::apply_blend_op(
						BlendPolicy::color_operation,
						src.green() * src_factor_color.green(),
						dst.green() * dst_factor_color.green() ),
					detail::apply_blend_op(
						BlendPolicy::color_operation,
						src.blue() * src_factor_color.blue(),
						dst.blue() * dst_factor_color.blue() ),
					detail::apply_blend_op(
						BlendPolicy::alpha_operation,
						src.alpha() * src_factor_alpha,
						dst.alpha() * dst_factor_alpha )
			};

			return result;
		}
	}

	// Apply color write mask
	template<typename ColorMaskPolicy>
	constexpr Color32 apply_color_mask( Color32 const& src, Color32 const& dst ) noexcept{
		if constexpr( ColorMaskPolicy::write_r &&
			ColorMaskPolicy::write_g &&
			ColorMaskPolicy::write_b &&
			ColorMaskPolicy::write_a ){
			return src;
		}
		else{
			return Color32{
				ColorMaskPolicy::write_r ? src.red() : dst.red(),
				ColorMaskPolicy::write_g ? src.green() : dst.green(),
				ColorMaskPolicy::write_b ? src.blue() : dst.blue(),
				ColorMaskPolicy::write_a ? src.alpha() : dst.alpha()
			};
		}
	}

	// Helper conversion functions
	constexpr ColorF to_colorf( Color32 const& c ) noexcept{
		return ColorF{ c };
	}

	constexpr Color32 to_color32( ColorF const& c ) noexcept{
		return static_cast< Color32 >( c );
	}

	// Raster State Policy Concept
	template<typename T>
	concept raster_state_policy = requires {
		// Blend state
		{ T::blend_enabled } -> std::convertible_to<bool>;
		{ T::src_color_factor } -> std::convertible_to<blend_factor>;
		{ T::dst_color_factor } -> std::convertible_to<blend_factor>;
		{ T::color_operation } -> std::convertible_to<blend_op>;
		{ T::src_alpha_factor } -> std::convertible_to<blend_factor>;
		{ T::dst_alpha_factor } -> std::convertible_to<blend_factor>;
		{ T::alpha_operation } -> std::convertible_to<blend_op>;

		// Depth state
		{ T::depth_test_enabled } -> std::convertible_to<bool>;
		{ T::depth_write_enabled } -> std::convertible_to<bool>;
		{ T::depth_function } -> std::convertible_to<depth_func>;

		// Raster state
		{ T::culling_mode } -> std::convertible_to<cull_mode>;
		{ T::front_face_winding } -> std::convertible_to<front_face>;

		// Color write mask
		{ T::write_r } -> std::convertible_to<bool>;
		{ T::write_g } -> std::convertible_to<bool>;
		{ T::write_b } -> std::convertible_to<bool>;
		{ T::write_a } -> std::convertible_to<bool>;
	};

	// Default raster state - matches current renderer behavior
	struct default_raster_state{
		// Blend state (disabled by default)
		static constexpr bool blend_enabled = false;
		static constexpr blend_factor src_color_factor = blend_factor::one;
		static constexpr blend_factor dst_color_factor = blend_factor::zero;
		static constexpr blend_op color_operation = blend_op::add;
		static constexpr blend_factor src_alpha_factor = blend_factor::one;
		static constexpr blend_factor dst_alpha_factor = blend_factor::zero;
		static constexpr blend_op alpha_operation = blend_op::add;

		// Depth state (enabled, less comparison)
		static constexpr bool depth_test_enabled = true;
		static constexpr bool depth_write_enabled = true;
		static constexpr depth_func depth_function = depth_func::less;

		// Raster state (backface culling, CW front face)
		static constexpr cull_mode culling_mode = cull_mode::back;
		static constexpr front_face front_face_winding = front_face::ccw;

		// Color write mask (all channels enabled)
		static constexpr bool write_r = true;
		static constexpr bool write_g = true;
		static constexpr bool write_b = true;
		static constexpr bool write_a = true;
	};

	// Alpha blending preset
	struct alpha_blend_raster_state{
		static constexpr bool blend_enabled = true;
		static constexpr blend_factor src_color_factor = blend_factor::src_alpha;
		static constexpr blend_factor dst_color_factor = blend_factor::inv_src_alpha;
		static constexpr blend_op color_operation = blend_op::add;
		static constexpr blend_factor src_alpha_factor = blend_factor::one;
		static constexpr blend_factor dst_alpha_factor = blend_factor::zero;
		static constexpr blend_op alpha_operation = blend_op::add;

		static constexpr bool depth_test_enabled = true;
		static constexpr bool depth_write_enabled = false; // Typically don't write depth for transparent objects
		static constexpr depth_func depth_function = depth_func::less_equal;

		static constexpr cull_mode culling_mode = cull_mode::none; // Often render both sides for transparency
		static constexpr front_face front_face_winding = front_face::cw;

		static constexpr bool write_r = true;
		static constexpr bool write_g = true;
		static constexpr bool write_b = true;
		static constexpr bool write_a = true;
	};

	// Additive blending preset
	struct additive_blend_raster_state{
		static constexpr bool blend_enabled = true;
		static constexpr blend_factor src_color_factor = blend_factor::src_alpha;
		static constexpr blend_factor dst_color_factor = blend_factor::one;
		static constexpr blend_op color_operation = blend_op::add;
		static constexpr blend_factor src_alpha_factor = blend_factor::zero;
		static constexpr blend_factor dst_alpha_factor = blend_factor::one;
		static constexpr blend_op alpha_operation = blend_op::add;

		static constexpr bool depth_test_enabled = true;
		static constexpr bool depth_write_enabled = false;
		static constexpr depth_func depth_function = depth_func::less_equal;

		static constexpr cull_mode culling_mode = cull_mode::none;
		static constexpr front_face front_face_winding = front_face::cw;

		static constexpr bool write_r = true;
		static constexpr bool write_g = true;
		static constexpr bool write_b = true;
		static constexpr bool write_a = true;
	};

	// No depth test preset
	struct no_depth_test_raster_state{
		static constexpr bool blend_enabled = false;
		static constexpr blend_factor src_color_factor = blend_factor::one;
		static constexpr blend_factor dst_color_factor = blend_factor::zero;
		static constexpr blend_op color_operation = blend_op::add;
		static constexpr blend_factor src_alpha_factor = blend_factor::one;
		static constexpr blend_factor dst_alpha_factor = blend_factor::zero;
		static constexpr blend_op alpha_operation = blend_op::add;

		static constexpr bool depth_test_enabled = false;
		static constexpr bool depth_write_enabled = false;
		static constexpr depth_func depth_function = depth_func::always;

		static constexpr cull_mode culling_mode = cull_mode::back;
		static constexpr front_face front_face_winding = front_face::cw;

		static constexpr bool write_r = true;
		static constexpr bool write_g = true;
		static constexpr bool write_b = true;
		static constexpr bool write_a = true;
	};

	// 2D rendering preset - alpha blending, no depth test, no culling
	// Use this for drawing 2D primitives, sprites, and text over a 3D scene.
	// Elements are drawn in submission order (last draw = on top).
	struct raster_2d_state{
		static constexpr bool blend_enabled = true;
		static constexpr blend_factor src_color_factor = blend_factor::src_alpha;
		static constexpr blend_factor dst_color_factor = blend_factor::inv_src_alpha;
		static constexpr blend_op color_operation = blend_op::add;
		static constexpr blend_factor src_alpha_factor = blend_factor::one;
		static constexpr blend_factor dst_alpha_factor = blend_factor::zero;
		static constexpr blend_op alpha_operation = blend_op::add;

		static constexpr bool depth_test_enabled = false;
		static constexpr bool depth_write_enabled = false;
		static constexpr depth_func depth_function = depth_func::always;

		static constexpr cull_mode culling_mode = cull_mode::none;
		static constexpr front_face front_face_winding = front_face::cw;

		static constexpr bool write_r = true;
		static constexpr bool write_g = true;
		static constexpr bool write_b = true;
		static constexpr bool write_a = true;
	};

	static_assert( raster_state_policy<default_raster_state> );
	static_assert( raster_state_policy<alpha_blend_raster_state> );
	static_assert( raster_state_policy<additive_blend_raster_state> );
	static_assert( raster_state_policy<no_depth_test_raster_state> );
	static_assert( raster_state_policy<raster_2d_state> );

	struct default_raster_state_no_cull{
		// Blend state (disabled by default)
		static constexpr bool blend_enabled = false;
		static constexpr blend_factor src_color_factor = blend_factor::one;
		static constexpr blend_factor dst_color_factor = blend_factor::zero;
		static constexpr blend_op color_operation = blend_op::add;
		static constexpr blend_factor src_alpha_factor = blend_factor::one;
		static constexpr blend_factor dst_alpha_factor = blend_factor::zero;
		static constexpr blend_op alpha_operation = blend_op::add;

		// Depth state (enabled, less comparison)
		static constexpr bool depth_test_enabled = true;
		static constexpr bool depth_write_enabled = true;
		static constexpr depth_func depth_function = depth_func::less;

		// Raster state (backface culling, CW front face)
		static constexpr cull_mode culling_mode = cull_mode::none;
		static constexpr front_face front_face_winding = front_face::ccw;

		// Color write mask (all channels enabled)
		static constexpr bool write_r = true;
		static constexpr bool write_g = true;
		static constexpr bool write_b = true;
		static constexpr bool write_a = true;
	};
}
