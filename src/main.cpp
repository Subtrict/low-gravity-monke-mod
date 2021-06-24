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
#include "config.hpp"
#include "LowGravityMonkeWatchView.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Register.hpp"
#include "custom-types/shared/register.hpp"

#include "GorillaLocomotion/Player.hpp"

#define INFO(value...) getLogger().info(value)
#define ERROR(value...) getLogger().error(value)

using namespace UnityEngine;
using namespace UnityEngine::XR;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}
float globalGravity = -9.81;
bool moddedRoom = false;

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

    if(config.isTrigger == true) {
        rightInput = OVRInput::Get(OVRInput::Button::PrimaryIndexTrigger, OVRInput::Controller::RTouch);
        leftInput = OVRInput::Get(OVRInput::Button::PrimaryIndexTrigger, OVRInput::Controller::LTouch);
    } else {
        rightInput = OVRInput::Get(OVRInput::Button::PrimaryHandTrigger, OVRInput::Controller::RTouch);
        leftInput = OVRInput::Get(OVRInput::Button::PrimaryHandTrigger, OVRInput::Controller::LTouch);
    }

    if(moddedRoom  && config.enabled) {  
        if(leftInput || rightInput) {
            playerPhysics->set_useGravity(false);
            UnityEngine::Vector3 gravity = globalGravity * config.gravityAmount * (UnityEngine::Vector3::get_up());
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
        moddedRoom = !CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(currentRoom, "get_IsVisible"));
    }
    else moddedRoom = true;

    // ? construction to switch what is logged, logs work like printf in C with the % placeholders
    getLogger().info("Room Joined! %s", moddedRoom ? "Room Was Private" : "Room Was not private");
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;

    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    GorillaUI::Init();

    INSTALL_HOOK_OFFSETLESS(getLogger(), GorillaTagManager_Update, il2cpp_utils::FindMethodUnsafe("", "GorillaTagManager", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PhotonNetworkController_OnJoinedRoom, il2cpp_utils::FindMethodUnsafe("", "PhotonNetworkController", "OnJoinedRoom", 0));
    
    custom_types::Register::RegisterType<LowGravityMonke::LowGravityMonkeWatchView>(); 
    GorillaUI::Register::RegisterWatchView<LowGravityMonke::LowGravityMonkeWatchView*>("Low Gravity Monke", VERSION);

    getLogger().info("Installing hooks...");
    // Install our hooks (none defined yet)
    getLogger().info("Installed all hooks!");
}