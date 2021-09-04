#pragma once
namespace bve {
    namespace script_wrappers {
        struct NamespacedName {
            MonoString* namespace_name;
            MonoString* local_name;
        };
        using Type = MonoReflectionType*;
        using IntPtr = void*;
        using string = MonoString*;

        double BasicVoxelEngine_Application_GetDeltaTime();

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

        void BasicVoxelEngine_Graphics_Factory_DestroyRef(IntPtr address);

        IntPtr BasicVoxelEngine_Model_LoadModel(string path, IntPtr factory);
        void BasicVoxelEngine_Model_DestroyRef(IntPtr address);

        string BasicVoxelEngine_AssetManager_GetAssetPath(string assetName);
    }
}