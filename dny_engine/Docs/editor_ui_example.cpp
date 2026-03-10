#include "../ui/dny_ui.hpp"

namespace{
	void editor_ui_frame( dny::ui::Panel& tools_panel, dny::Mouse const& mouse, dny::Keyboard& keyboard, dny::surface<dny::Color32>& canvas, dny::Font const& font ){
		tools_panel.update( mouse, keyboard );

		if( auto* raw_save = dynamic_cast<dny::ui::Button*>( tools_panel.find_child( "save" ) ); raw_save && raw_save->was_clicked() ){
			// save_level();
		}

		if( auto* raw_snap = dynamic_cast<dny::ui::CheckBox*>( tools_panel.find_child( "snap" ) ); raw_snap && raw_snap->was_toggled() ){
			// set_grid_snap( raw_snap->checked() );
		}

		tools_panel.draw( canvas, font );
	}
}
