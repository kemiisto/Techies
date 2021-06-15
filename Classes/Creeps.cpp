#include "Creeps.h"

#include <numeric>

#include "CreepFactory.h"
#include "GameScene.h"

using namespace cocos2d;

const std::map<Creep::Type, std::size_t> maxCreepsOfType = {
	{Creep::Type::Melee,  20},
	{Creep::Type::Ranged, 10},
	{Creep::Type::Siege,   5}
};

const std::map<Creep::Type, float> creepScale = {
    {Creep::Type::Melee,  0.800f},
    {Creep::Type::Ranged, 0.750f},
    {Creep::Type::Siege,  0.600f}
};

Creeps::Creeps(GameScene* scene, const std::map<Creep::Type, int>& creepValue, const std::map<Creep::Type, float>& creepDamage) :
	    creeps{
	        {Creep::Type::Melee,  {} },
	        {Creep::Type::Ranged, {} },
	        {Creep::Type::Siege,  {} }
		},
		idx{
	        {Creep::Type::Melee,  0 },
	        {Creep::Type::Ranged, 0 },
	        {Creep::Type::Siege,  0 }
		},
		maxCreeps(0),
		creepValue(creepValue),
		creepDamage(creepDamage) {
    createCreeps(scene, Creep::Type::Melee);
    createCreeps(scene, Creep::Type::Ranged);
    createCreeps(scene, Creep::Type::Siege);
    maxCreeps = std::accumulate(
        maxCreepsOfType.begin(),
        maxCreepsOfType.end(),
        0,
        [](const std::size_t previous, const auto& element) {
            return previous + element.second;
        }
    );
}

std::size_t Creeps::size() const {
    return maxCreeps;
}

Creep* Creeps::get(const Creep::Type creepType) {
    auto& i = idx.at(creepType);
    auto creep = creeps.at(creepType).at(i);
    i++;
    if (i == creeps.at(creepType).size()) {
        i = 0;
    }
    return creep;
}

void Creeps::createCreeps(GameScene* scene, const Creep::Type& creepType) {
    for (std::size_t i = 0; i < maxCreepsOfType.at(creepType); ++i) {
        auto creep = CreepFactory::create(creepType, creepValue.at(creepType), creepDamage.at(creepType));
        creep->setVisible(false);
        creep->setScale(creepScale.at(creepType));
        creeps.at(creepType).push_back(creep);
        scene->addChild(creep, 1);
    }
}