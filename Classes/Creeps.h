#ifndef TECHIES_CREEPS_H
#define TECHIES_CREEPS_H

#include "Creep.h"

class GameScene;
class Creeps {
public:
	Creeps(GameScene* scene, const std::map<Creep::Type, int>& creepValue, const std::map<Creep::Type, float>& creepDamage);
	std::size_t size() const;
	Creep* get(Creep::Type creepType);
private:
	std::map<Creep::Type, std::vector<Creep*>> creeps;
	std::map<Creep::Type, std::size_t> idx;
	std::size_t maxCreeps;
	const std::map<Creep::Type, int>& creepValue;
	const std::map<Creep::Type, float>& creepDamage;
	void createCreeps(GameScene* scene, const Creep::Type& creepType);
};

#endif // TECHIES_CREEPS_H