#ifndef TECHIES_MELEE_CREEP_H
#define TECHIES_MELEE_CREEP_H

#include "Creep.h"

class MeleeCreep final : public Creep {
    friend class CreepFactory;
public:
    int value() const override;
    int damage() const override;
private:
    MeleeCreep();
};


#endif // TECHIES_MELEE_CREEP_H
