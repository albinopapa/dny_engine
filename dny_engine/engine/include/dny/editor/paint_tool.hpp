#pragma once

#include "editor/level_editor.hpp"
#include "editor/basic_tool.hpp"

#include <optional>

namespace dny{
	class LevelEditor::PaintTool : public basic_tool{
	public:
		PaintTool(LevelEditor& parent_) noexcept;
		void update( Mouse const& mouse_, Keyboard& keyboard_ ) override;
		//void render( renderer2d& renderer_ ) const override;
	private:
		LevelEditor& m_parent;
		std::optional< vector2<std::int32_t> > m_start_tile;
	};
}