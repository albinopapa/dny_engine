#pragma once

#include "physics/aabb.hpp"
#include "math/math.hpp"

class lock_on_camera{
public:
	void update( dny::vector2<float> const& target_position, dny::aabb<float> const& world_bounds ) noexcept{
		position.x = std::clamp( target_position.x, world_bounds.min_pt.x, world_bounds.max_pt.x );
		position.y = std::clamp( target_position.y, world_bounds.min_pt.y, world_bounds.max_pt.y );
	}

	void zoom_in( float amount_degrees ) noexcept{
		fov = std::clamp( fov - amount_degrees, min_fov, max_fov );
	}

	void zoom_out( float amount_degrees ) noexcept{
		fov = std::clamp( fov + amount_degrees, min_fov, max_fov );
	}

	dny::matrix_4x4<float> get_view_matrix() const noexcept{
		return dny::look_to<float, dny::handedness_t::left>(
			position,
			forward,
			up
		);
	}

	float get_fov() const noexcept{
		return dny::to_radians( fov );
	}

	const dny::vector3<float>& get_position() const noexcept{
		return position;
	}

private:
	static constexpr float min_fov = 30.f;
	static constexpr float max_fov = 90.f;
	static constexpr dny::vector3<float> forward{ 0.f, 0.f, 1.f };
	static constexpr dny::vector3<float> up{ 0.f, 1.f, 0.f };

	dny::vector3<float> position{ 0.f, 0.f, -10.f };
	float fov = 90.f;
};

