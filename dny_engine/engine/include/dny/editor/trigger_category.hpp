#pragma once

#include "utilities/rectangle.hpp"

#include <cstdint>

namespace dny{
	enum class TriggerTargetType : int32_t{
		none = 0,
		tile = 1,
		entity = 2,
		object = 3,
	};

	enum class TriggerConditionType : int32_t{
		invalid = 0,
		entity_in_region = 1,
		room_cleared = 2,
		item_acquired = 3,
		time_expired = 4,
		flag_set = 5,
	};

	enum class TriggerActionType : int32_t{
		invalid = 0,
		unlock_door = 1,
		set_tile_state = 2,
		play_animation = 3,
		spawn_entity = 4,
		toggle_light = 5,
	};

	// ---------- ENUM CONVERSION HELPERS ----------

	constexpr TriggerTargetType to_trigger_target_type( int32_t v ) noexcept{
		switch( v ){
			case 1: return TriggerTargetType::tile;
			case 2: return TriggerTargetType::entity;
			case 3: return TriggerTargetType::object;
			default: return TriggerTargetType::none;
		}
	}

	constexpr TriggerConditionType to_trigger_condition_type( int32_t v ) noexcept{
		switch( v ){
			case 1: return TriggerConditionType::entity_in_region;
			case 2: return TriggerConditionType::room_cleared;
			case 3: return TriggerConditionType::item_acquired;
			case 4: return TriggerConditionType::time_expired;
			case 5: return TriggerConditionType::flag_set;
			default: return TriggerConditionType::invalid;
		}
	}

	constexpr TriggerActionType to_trigger_action_type( int32_t v ) noexcept{
		switch( v ){
			case 1: return TriggerActionType::unlock_door;
			case 2: return TriggerActionType::set_tile_state;
			case 3: return TriggerActionType::play_animation;
			case 4: return TriggerActionType::spawn_entity;
			case 5: return TriggerActionType::toggle_light;
			default: return TriggerActionType::invalid;
		}
	}

	// ---------- STRUCTURES ----------

	struct TriggerTarget{
		TriggerTargetType type = TriggerTargetType::none;
		int32_t id = 0;   // tile ID, entity ID, platform ID, etc.
	};

	struct TriggerCondition{
		TriggerConditionType type = TriggerConditionType::invalid;
		Rect<float> region;   // only used by some conditions
		int32_t id = 0;   // flag ID, item ID, room ID, etc.
	};

	struct TriggerAction{
		TriggerActionType type = TriggerActionType::invalid;
		TriggerTarget target;
		int32_t id = 0;   // animation ID, tile-state, entity-type-to-spawn, etc.
	};

} // namespace dny
