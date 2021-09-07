using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public static class AssetManager
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetAssetPath(string assetName);
    }
}