#pragma once
namespace bve {
    namespace script_wrappers {
        double BasicVoxelEngine_Application_GetDeltaTime();

        void BasicVoxelEngine_Logger_PrintDebug(MonoString* message);
        void BasicVoxelEngine_Logger_PrintInfo(MonoString* message);
        void BasicVoxelEngine_Logger_PrintWarning(MonoString* message);
        void BasicVoxelEngine_Logger_PrintError(MonoString* message);
    }
}