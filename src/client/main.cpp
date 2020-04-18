#include "client/engine.h"
int main(){

	Engine *engineInstance = new Engine("Test Engine",1280,720,90);
	engineInstance->ENABLE_VALIDATION_LAYERS=false;
	engineInstance->initVulkan();
	engineInstance->initTerrain(6);
	engineInstance->mainLoop();
	delete engineInstance;

}
