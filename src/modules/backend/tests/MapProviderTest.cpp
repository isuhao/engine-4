/**
 * @file
 */

#include "core/tests/AbstractTest.h"
#include "backend/world/MapProvider.h"
#include "network/ProtocolHandlerRegistry.h"
#include "network/ServerNetwork.h"
#include "network/ServerMessageSender.h"
#include "cooldown/CooldownProvider.h"
#include "attrib/ContainerProvider.h"
#include "backend/entity/ai/AIRegistry.h"
#include "backend/entity/ai/AILoader.h"
#include "backend/entity/EntityStorage.h"
#include "voxel/MaterialColor.h"

namespace backend {

class MapProviderTest: public core::AbstractTest {
public:
	EntityStoragePtr _entityStorage;
	network::ProtocolHandlerRegistryPtr _protocolHandlerRegistry;
	network::ServerNetworkPtr _network;
	network::ServerMessageSenderPtr _messageSender;
	AILoaderPtr _loader;
	attrib::ContainerProviderPtr _containerProvider;
	cooldown::CooldownProviderPtr _cooldownProvider;

	void SetUp() override {
		core::AbstractTest::SetUp();
		core::Var::get(cfg::ServerSeed, "1");
		core::Var::get(cfg::VoxelMeshSize, "16", core::CV_READONLY);
		voxel::initDefaultMaterialColors();
		_entityStorage = std::make_shared<EntityStorage>(_testApp->eventBus());
		_protocolHandlerRegistry = std::make_shared<network::ProtocolHandlerRegistry>();
		_network = std::make_shared<network::ServerNetwork>(_protocolHandlerRegistry, _testApp->eventBus());
		_messageSender = std::make_shared<network::ServerMessageSender>(_network);
		const AIRegistryPtr& registry = std::make_shared<AIRegistry>();
		registry->init();
		_loader = std::make_shared<AILoader>(registry);
		_containerProvider = std::make_shared<attrib::ContainerProvider>();
		_cooldownProvider = std::make_shared<cooldown::CooldownProvider>();
	}
};

#define create(name) \
	MapProvider name(_testApp->filesystem(), _testApp->eventBus(), _testApp->timeProvider(), \
			_entityStorage, _messageSender, _loader, _containerProvider, _cooldownProvider);

TEST_F(MapProviderTest, testInitShutdown) {
	create(provider);
	EXPECT_TRUE(provider.init()) << "Failed to initialize the map provider";
	provider.shutdown();
}

TEST_F(MapProviderTest, testCreateMap) {
	create(provider);
	EXPECT_TRUE(provider.init()) << "Failed to initialize the map provider";
	EXPECT_FALSE(provider.worldMaps().empty());
	const MapPtr& map = provider.map(1, false);
	EXPECT_TRUE(map);
	provider.shutdown();
}

}
