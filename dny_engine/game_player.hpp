#pragma once

#include "dny_input.hpp"
#include "dny_math.hpp"
#include "dny_physics.hpp"

static constexpr float action_plane_z = 5.f;

class Player{
public:
	dny::vector2<float> const& get_position()const{
		return position;
	}
	void update( float dt, dny::input const& input, dny::polyline_collider<float> const& terrain_ ){
		const auto move_dir =
			( input.is_held( "move_right" ) ? 1.f : 0.f ) -
			( input.is_held( "move_left" ) ? 1.f : 0.f );

		position.x += move_dir * move_speed * dt;

		if( input.is_pressed( "jump" ) && is_on_ground ){
			velocity.y = jump_velocity;
			is_on_ground = false;
		}

		velocity.y += gravity * dt;
		position.y += velocity.y * dt;

		auto player_bounds = dny::aabb<float>{
			{
				position.x - size.width * 0.5f,
				position.y - size.height * 0.5f,
				action_plane_z - 0.5f
			},
			{
				position.x + size.width * 0.5f,
				position.y + size.height * 0.5f,
				action_plane_z + 0.5f
			}
		};

		const auto resolution = dny::resolve_aabb_vs_polyline( player_bounds, terrain_, 0.75f );
		if( resolution.y != 0.f ){
			position.y += resolution.y;
			velocity.y = 0.f;
			is_on_ground = true;
		}
		else if( position.y < fallback_ground_height ){
			position.y = fallback_ground_height;
			velocity.y = 0.f;
			is_on_ground = true;
		}
		else{
			is_on_ground = false;
		}
	}
	dny::matrix_4x4<float> get_transform()const{
		auto rotation = dny::matrix_4x4<float>::rotation_x( dny::to_radians( 90.f ) );
		auto translation = dny::matrix_4x4<float>::translation( { position.x, position.y, action_plane_z } );
		auto scaling = dny::matrix_4x4<float>::scaling( { size.width, size.height, 1.f } );
		return rotation * scaling * translation;
	}
private:
	static constexpr float move_speed = 12.f;
	static constexpr float jump_velocity = 16.f;
	static constexpr float gravity = -36.f;
	static constexpr float fallback_ground_height = -4.f;
	static constexpr float dim = 4.f;
	static constexpr dny::dims2<float> size{ dim, dim };
	dny::vector2<float> position{ 0.f, fallback_ground_height };
	dny::vector2<float> velocity{ 0.f, 0.f };
	bool is_on_ground = true;

};

