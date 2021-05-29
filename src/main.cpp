#include <thread>
#include "main.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "GlobalNamespace/OVRInput_Button.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/ForceMode.hpp"

#include "GorillaLocomotion/Player.hpp"

using namespace UnityEngine;
using namespace UnityEngine::XR;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}
float globalGravity = -9.81;
bool allowLowGravity = false;

#include "GlobalNamespace/GorillaTagManager.hpp"
MAKE_HOOK_OFFSETLESS(GorillaTagManager_Update, void, GlobalNamespace::GorillaTagManager* self) {
    using namespace GlobalNamespace;
    using namespace GorillaLocomotion;

    Player* playerInstance = Player::get_Instance();
    if(playerInstance == nullptr) return;

    Rigidbody* playerPhysics = playerInstance->playerRigidBody;
    if(playerPhysics == nullptr) return;

    bool rightInput = false;
    bool leftInput = false;

    if(getConfig().config["isTrigger"].GetBool() == true) {
        rightInput = OVRInput::Get(OVRInput::Button::PrimaryIndexTrigger, OVRInput::Controller::RTouch);
        leftInput = OVRInput::Get(OVRInput::Button::PrimaryIndexTrigger, OVRInput::Controller::LTouch);
    } else {
        rightInput = OVRInput::Get(OVRInput::Button::PrimaryHandTrigger, OVRInput::Controller::RTouch);
        leftInput = OVRInput::Get(OVRInput::Button::PrimaryHandTrigger, OVRInput::Controller::LTouch);
    }

    if(allowLowGravity) {  
        if(leftInput || rightInput) {
            playerPhysics->set_useGravity(false);
            UnityEngine::Vector3 gravity = globalGravity * getConfig().config["gravityAmount"].GetFloat() * (UnityEngine::Vector3::get_up());
            playerPhysics->AddForce(gravity, UnityEngine::ForceMode::Acceleration);
        }
        else {
            playerPhysics->set_useGravity(true);
        } 
    }
    else {
        playerPhysics->set_useGravity(true);
    }


    GorillaTagManager_Update(self);
}

#include "GlobalNamespace/PhotonNetworkController.hpp"

MAKE_HOOK_OFFSETLESS(PhotonNetworkController_OnJoinedRoom, void, Il2CppObject* self)
{
    PhotonNetworkController_OnJoinedRoom(self);

    Il2CppObject* currentRoom = CRASH_UNLESS(il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "get_CurrentRoom"));

    if (currentRoom)
    {
        // get wether or not this is a private room
        allowLowGravity = !CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(currentRoom, "get_IsVisible"));
    }
    else allowLowGravity = true;

    // ? construction to switch what is logged, logs work like printf in C with the % placeholders
    getLogger().info("Room Joined! %s", allowLowGravity ? "Room Was Private" : "Room Was not private");
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load();
    rapidjson::Document::AllocatorType& allocator = getConfig().config.GetAllocator();
    if (!getConfig().config.HasMember("isTrigger")) {
        getConfig().config.AddMember("isTrigger", rapidjson::Value(0).SetBool(false), allocator);
        getConfig().Write();
    }
    if (!getConfig().config.HasMember("gravityAmount")) {
        getConfig().config.AddMember("gravityAmount", rapidjson::Value(0).SetFloat(0.5f), allocator);
        getConfig().Write();
    }
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    INSTALL_HOOK_OFFSETLESS(getLogger(), GorillaTagManager_Update, il2cpp_utils::FindMethodUnsafe("", "GorillaTagManager", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PhotonNetworkController_OnJoinedRoom, il2cpp_utils::FindMethodUnsafe("", "PhotonNetworkController", "OnJoinedRoom", 0));
    getLogger().info("Installing hooks...");
    // Install our hooks (none defined yet)
    getLogger().info("Installed all hooks!");
}