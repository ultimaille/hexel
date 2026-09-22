#pragma once
#include "core.h"

// -------------------------------------------------------------------------------
//                                    Panels to explore things (layers/xcf/etc.)
// -------------------------------------------------------------------------------
// => are more or less independant of the mode
struct Panel{
    virtual ~Panel() = default;
	virtual void generate_gui()=0;
};
struct PanelManager{
	Registry<Panel> panels;
	void show_gui(){
		for(auto& [name,obj] : panels) obj->generate_gui();
	}
};


