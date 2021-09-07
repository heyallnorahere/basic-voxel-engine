using System;

namespace BasicVoxelEngine
{
    /// <summary>
    /// This class is for managed helpers to native code, i.e. getting generic types
    /// </summary>
    public static class Helpers
    {
        public static Type GetGenericType(Type baseType, Type[] @params)
        {
            return baseType.MakeGenericType(@params);
        }
    }
}