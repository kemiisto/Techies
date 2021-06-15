#ifndef TECHIES_RANGED_CREEP_H
#define TECHIES_RANGED_CREEP_H

#include "Creep.h"

class RangedCreep final : public Creep {
    friend class CreepFactory;
private:
    RangedCreep(int value, int damage);
};

#endif // TECHIES_RANGED_CREEP_H
