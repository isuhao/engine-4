/**
 * @file
 */

#include "SpawnMgr.h"
#include "core/Common.h"
#include "core/Singleton.h"
#include "core/App.h"
#include "io/Filesystem.h"

namespace backend {

static const long spawnTime = 15000L;

SpawnMgr::SpawnMgr(voxel::WorldPtr world, EntityStoragePtr entityStorage, network::MessageSenderPtr messageSender, core::TimeProviderPtr timeProvider, AILoaderPtr loader, attrib::ContainerProviderPtr containerProvider, PoiProviderPtr poiProvider) :
		_loader(loader), _world(world), _entityStorage(entityStorage), _messageSender(messageSender), _timeProvider(timeProvider), _containerProvider(containerProvider), _poiProvider(poiProvider), _time(15000L) {
}

void SpawnMgr::shutdown() {
}

bool SpawnMgr::init() {
	const std::string& lua = core::App::getInstance()->filesystem()->load("behaviourtrees.lua");
	if (!_loader->init(lua)) {
		Log::error("could not load the behaviourtrees: %s", _loader->getError().c_str());
		return false;
	}
	return true;
}

void SpawnMgr::spawnCharacters(ai::Zone& zone) {
	spawnEntity(zone, network::messages::EntityType::BEGIN_CHARACTERS, network::messages::EntityType::MAX_CHARACTERS, 0);
}

void SpawnMgr::spawnAnimals(ai::Zone& zone) {
	spawnEntity(zone, network::messages::EntityType::BEGIN_ANIMAL, network::messages::EntityType::MAX_ANIMAL, 2);
}

void SpawnMgr::spawnEntity(ai::Zone& zone, network::messages::EntityType start, network::messages::EntityType end, int maxAmount) {
	const int offset = (int)start + 1;
	int count[(int)end - offset];
	memset(count, 0, sizeof(count));
	zone.execute([start, end, offset, &count] (const ai::AIPtr& ai) {
		const AICharacter& chr = ai::character_cast<AICharacter>(ai->getCharacter());
		const Npc& npc = chr.getNpc();
		const network::messages::EntityType type = npc.npcType();
		if (type <= start || type >= end) {
			return;
		}
		const int index = (int)type - offset;
		++count[index];
	});

	const int size = SDL_arraysize(count);
	for (int i = 0; i < size; ++i) {
		if (count[i] >= maxAmount)
			continue;

		const int needToSpawn = maxAmount - count[i];
		network::messages::EntityType type = static_cast<network::messages::EntityType>(offset + i);
		spawn(zone, type, needToSpawn);
	}
}

int SpawnMgr::spawn(ai::Zone& zone, network::messages::EntityType type, int amount, const glm::ivec3* pos) {
	const char *typeName = network::messages::EnumNameEntityType(type);
	ai::TreeNodePtr behaviour = _loader->load(typeName);
	if (!behaviour) {
		Log::error("could not load the behaviour tree %s", typeName);
		return 0;
	}
	for (int x = 0; x < amount; ++x) {
		const NpcPtr& npc = std::make_shared<Npc>(type, _entityStorage, behaviour, _world, _messageSender, _timeProvider, _containerProvider, _poiProvider);
		npc->init(pos);
		// now let it tick
		zone.addAI(npc->ai());
		_entityStorage->addNpc(npc);
	}

	return amount;
}

void SpawnMgr::onFrame(ai::Zone& zone, long dt) {
	_time += dt;
	if (_time >= spawnTime) {
		_time -= spawnTime;
		spawnAnimals(zone);
		spawnCharacters(zone);
	}
}

}
