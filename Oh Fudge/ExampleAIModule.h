#include <BWAPI.h>
#define UN using namespace
UN BWAPI; 
UN Filter; 
UN std; 
using U = Unit; 
auto&g = BroodwarPtr; 

struct ExampleAIModule :AIModule { 
	void onFrame() { 
		g->drawTextScreen(0, 0, "Nice try");
	}
};