#ifndef TECHIES_SIEGE_CREEP_H
#define TECHIES_SIEGE_CREEP_H

#include "Creep.h"

class SiegeCreep final : public Creep {
    friend class CreepFactory;
private:
    SiegeCreep(int value, int damage);
};

#endif // TECHIES_SIEGE_CREEP_H
