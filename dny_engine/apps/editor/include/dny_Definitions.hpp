#pragma once

#include "dny_TileCategory.hpp"
#include "dny_TriggerCategory.hpp"
#include "math/math.hpp"

#include <cstdint>
#include <string>

namespace dny{
	struct TileDef{
		bool operator==( TileDef const& other )const noexcept{
			return definition_id == other.definition_id;
		}
		bool operator!=( TileDef const& other )const noexcept{
			return !( *this == other );
		}

		std::string name;
		std::string texture_name;
		TileCategory category = TileCategory::Empty;
		float friction = 0.f;
		float damage = 0.f;
		std::int32_t definition_id = 0;
		std::int32_t platform_id = 0;
		std::int32_t entity_id = 0;
		std::int32_t trigger_id = 0;
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
}
