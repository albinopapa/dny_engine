#pragma once

#include "tile_category.hpp"
#include "trigger_category.hpp"
#include "math/math.hpp"
#include "physics/physics.hpp"

#include <cstdint>
#include <string>

namespace dny{

	struct TileDef{
		std::string_view name;
		std::string_view texture_name;
		TileCategory category = TileCategory::Empty;
		float friction = 0.f;
		float damage = 0.f;

		// Add a default constructor to allow aggregate initialization in std::vector
		constexpr TileDef() = default;
		constexpr TileDef(std::string_view n, std::string_view tex, TileCategory cat, float fric, float dmg)
			: name(n), texture_name(tex), category(cat), friction(fric), damage(dmg) {}
	};

	struct PlatformDef{
		bool operator==( PlatformDef const& other )const noexcept{
			return id == other.id;
		}
		bool operator!=( PlatformDef const& other )const noexcept{
			return !( *this == other );
		}

		std::string texture_name;
		vector2<float> start;
		vector2<float> end;
		dims2<float> size;
		float speed = 0.f;
		std::int32_t id = 0;
		bool is_moveable = false;
	};

	struct EntityDef{
		std::string name;
		vector2<float> spawn_pos;
		dims2<float> size;
		float max_speed = 0.f;
		float damage = 0.f;
		float health = 100.f;
		std::int32_t id = 0;
	};

	struct TriggerDef{
		TriggerCondition condition;
		TriggerAction action;
		std::int32_t id = 0;       // unique trigger ID in the level
		bool fire_once = true;
	};

	struct Polyline{
		void add_point( vector2<float> const& point ){
			collider.points.push_back( point );
		}
		void delete_last_point(){
			if( !collider.points.empty() ){
				collider.points.pop_back();
			}
		}
		void delete_point(std::size_t index ){
			if( index < collider.points.size() ){
				collider.points.erase( collider.points.begin() + index );
			}
		}

		polyline_collider<float> collider;
	};
}
