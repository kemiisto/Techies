#ifndef TECHIES_CREEP_FACTORY_H
#define TECHIES_CREEP_FACTORY_H

#include "Creep.h"

class CreepFactory {
public:
	static Creep* create(Creep::Type creepType);
};

#endif // TECHIES_CREEP_FACTORY_H
