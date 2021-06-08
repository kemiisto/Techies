#ifndef TECHIES_RANGED_CREEP_H
#define TECHIES_RANGED_CREEP_H

#include "Creep.h"

class RangedCreep final : public Creep {
    friend class Creep;
public:
    int value() const override;
    int damage() const override;
private:
    RangedCreep();
};

#endif // TECHIES_RANGED_CREEP_H
