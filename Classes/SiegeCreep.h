#ifndef TECHIES_SIEGE_CREEP_H
#define TECHIES_SIEGE_CREEP_H

#include "Creep.h"

class SiegeCreep final : public Creep {
    friend class Creep;
public:
    int value() const override;
    int damage() const override;
private:
    SiegeCreep();
};

#endif // TECHIES_SIEGE_CREEP_H
