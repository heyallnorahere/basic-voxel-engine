#pragma once
#include "lighting/light.h"
#include "input_manager.h"
namespace bve {
    namespace script_wrappers {
        struct NamespacedName {
            MonoString* namespace_name;
            MonoString* local_name;
        };
        using Type = MonoReflectionType*;
        using IntPtr = void*;
        using string = MonoString*;
        using Vector3 = glm::vec3;
        using Vector3I = glm::ivec3;
        using LightType = lighting::light_type;
        using uint = uint32_t;
        using object = MonoObject*;
        using Vector2 = glm::vec2;

        double BasicVoxelEngine_Application_GetDeltaTime();
        IntPtr BasicVoxelEngine_Application_GetWorld();
        IntPtr BasicVoxelEngine_Application_GetInputManager();

        void BasicVoxelEngine_Logger_PrintDebug(string message);
        void BasicVoxelEngine_Logger_PrintInfo(string message);
        void BasicVoxelEngine_Logger_PrintWarning(string message);
        void BasicVoxelEngine_Logger_PrintError(string message);

        void BasicVoxelEngine_Registry_RegisterTypes();
        bool BasicVoxelEngine_Registry_RegisterExists(Type type);
        IntPtr BasicVoxelEngine_Registry_CreateRegisterRef(Type type);

        IntPtr BasicVoxelEngine_Register_CreateRef(int32_t index, Type type, IntPtr address);
        int32_t BasicVoxelEngine_Register_GetCount(Type type, IntPtr address);
        bool BasicVoxelEngine_Register_GetIndex(NamespacedName namespacedName, Type type, IntPtr address, int32_t* index);
        bool BasicVoxelEngine_Register_GetNamespacedName(int32_t index, Type type, IntPtr address, NamespacedName* namespacedName);
        int32_t BasicVoxelEngine_Register_RegisterObject(MonoObject* object, NamespacedName namespacedName, Type type, IntPtr address);
        bool BasicVoxelEngine_Register_IsManaged(int32_t index, Type type, IntPtr address);
        MonoObject* BasicVoxelEngine_Register_GetManagedObject(int32_t index, Type type, IntPtr address);

        void BasicVoxelEngine_RegisteredObject_DestroyRef(IntPtr nativeAddress);

        float BasicVoxelEngine_Block_GetOpacity(IntPtr nativeAddress);
        bool BasicVoxelEngine_Block_GetSolid(IntPtr nativeAddress);
        string BasicVoxelEngine_Block_GetFriendlyName(IntPtr nativeAddress);
        IntPtr BasicVoxelEngine_Block_GetModel(IntPtr nativeAddress);
        IntPtr BasicVoxelEngine_Block_GetLight(IntPtr nativeAddress, LightType* type);

        void BasicVoxelEngine_Graphics_Factory_DestroyRef(IntPtr address);

        IntPtr BasicVoxelEngine_Model_LoadModel(string path, IntPtr factory);
        void BasicVoxelEngine_Model_DestroyRef(IntPtr address);

        string BasicVoxelEngine_AssetManager_GetAssetPath(string assetName);

        void BasicVoxelEngine_Lighting_Light_Destroy(IntPtr address);
        void BasicVoxelEngine_Lighting_Light_SetColor(IntPtr address, Vector3 color);
        void BasicVoxelEngine_Lighting_Light_SetAmbientStrength(IntPtr address, float strength);
        void BasicVoxelEngine_Lighting_Light_SetSpecularStrength(IntPtr address, float strength);
        LightType BasicVoxelEngine_Lighting_Light_GetType(IntPtr address);

        IntPtr BasicVoxelEngine_Lighting_Spotlight_Create();
        void BasicVoxelEngine_Lighting_Spotlight_SetDirection(IntPtr address, Vector3 direction);
        void BasicVoxelEngine_Lighting_Spotlight_SetCutoff(IntPtr address, float cutoff);

        IntPtr BasicVoxelEngine_Lighting_PointLight_Create();
        void BasicVoxelEngine_Lighting_PointLight_SetConstant(IntPtr address, float constant);
        void BasicVoxelEngine_Lighting_PointLight_SetLinear(IntPtr address, float linear);
        void BasicVoxelEngine_Lighting_PointLight_SetQuadratic(IntPtr address, float quadratic);

        void BasicVoxelEngine_World_Destroy(IntPtr address);
        int32_t BasicVoxelEngine_World_GetBlock(IntPtr address, Vector3I position);
        void BasicVoxelEngine_World_SetBlock(IntPtr address, Vector3I position, int32_t index);
        void BasicVoxelEngine_World_AddOnBlockChangedCallback(IntPtr address, MonoObject* callback);

        object BasicVoxelEngine_Entity_AddComponent(uint id, IntPtr world, Type type);
        object BasicVoxelEngine_Entity_GetComponent(uint id, IntPtr world, Type type);
        bool BasicVoxelEngine_Entity_HasComponent(uint id, IntPtr world, Type type);
        void BasicVoxelEngine_Entity_RemoveComponent(uint id, IntPtr world, Type type);
        void BasicVoxelEngine_Entity_RegisterComponents();

        Vector3 BasicVoxelEngine_Components_TransformComponent_GetTranslation(IntPtr address);
        void BasicVoxelEngine_Components_TransformComponent_SetTranslation(IntPtr address, Vector3 value);
        Vector3 BasicVoxelEngine_Components_TransformComponent_GetRotation(IntPtr address);
        void BasicVoxelEngine_Components_TransformComponent_SetRotation(IntPtr address, Vector3 value);
        Vector3 BasicVoxelEngine_Components_TransformComponent_GetScale(IntPtr address);
        void BasicVoxelEngine_Components_TransformComponent_SetScale(IntPtr address, Vector3 value);

        Vector3 BasicVoxelEngine_Components_CameraComponent_GetDirection(IntPtr address);
        void BasicVoxelEngine_Components_CameraComponent_SetDirection(IntPtr address, Vector3 value);
        Vector3 BasicVoxelEngine_Components_CameraComponent_GetUp(IntPtr address);
        void BasicVoxelEngine_Components_CameraComponent_SetUp(IntPtr address, Vector3 value);
        bool BasicVoxelEngine_Components_CameraComponent_GetPrimary(IntPtr address);
        void BasicVoxelEngine_Components_CameraComponent_SetPrimary(IntPtr address, bool value);
        float BasicVoxelEngine_Components_CameraComponent_GetNearPlane(IntPtr address);
        void BasicVoxelEngine_Components_CameraComponent_SetNearPlane(IntPtr address, float value);
        float BasicVoxelEngine_Components_CameraComponent_GetFarPlane(IntPtr address);
        void BasicVoxelEngine_Components_CameraComponent_SetFarPlane(IntPtr address, float value);

        void BasicVoxelEngine_InputManager_DestroyRef(IntPtr address);
        bool BasicVoxelEngine_InputManager_GetMouseEnabled(IntPtr address);
        void BasicVoxelEngine_InputManager_SetMouseEnabled(IntPtr address, bool value);
        Vector2 BasicVoxelEngine_InputManager_GetMouse(IntPtr address);
        input_manager::key_state BasicVoxelEngine_InputManager_GetKey(IntPtr address, int32_t key);
    }
}