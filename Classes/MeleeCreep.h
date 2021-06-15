#ifndef TECHIES_MELEE_CREEP_H
#define TECHIES_MELEE_CREEP_H

#include "Creep.h"

class MeleeCreep final : public Creep {
    friend class CreepFactory;
private:
    MeleeCreep(int value, int damage);
};


#endif // TECHIES_MELEE_CREEP_H
