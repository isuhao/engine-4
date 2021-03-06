/**
 * @file
 */

#pragma once

#include "backend/ForwardDecl.h"
#include "ai/common/Types.h"
#include "core/EventBus.h"
#include "backend/eventbus/Event.h"
#include <unordered_map>

namespace backend {

/**
 * @brief Manages the Entity instances of the backend.
 *
 * This includes calling the Entity::update() method as well as performing the visibility calculations.
 */
class EntityStorage : public core::IEventBusHandler<EntityDeleteEvent>{
private:
	typedef std::unordered_map<EntityId, UserPtr> Users;
	typedef Users::iterator UsersIter;
	Users _users;

	typedef std::unordered_map<EntityId, NpcPtr> Npcs;
	typedef Npcs::iterator NpcsIter;
	Npcs _npcs;

	core::EventBusPtr _eventBus;
	long _time;

	bool updateEntity(const EntityPtr& entity, long dt);
public:
	EntityStorage(const core::EventBusPtr& eventBus);

	void onEvent(const EntityDeleteEvent& event);

	bool addUser(const UserPtr& user);
	bool removeUser(EntityId userId);
	UserPtr user(EntityId userId);

	bool addNpc(const NpcPtr& npc);
	bool removeNpc(EntityId id);
	NpcPtr npc(EntityId id);
};

typedef std::shared_ptr<EntityStorage> EntityStoragePtr;

}
