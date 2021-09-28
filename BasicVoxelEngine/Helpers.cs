using System;
using System.Collections.Generic;

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
        public static bool DerivesFrom(this Type type, Type baseType)
        {
            Type? currentType = type;
            if (type == baseType)
            {
                return true;
            }
            while (currentType != null)
            {
                currentType = currentType.BaseType;
                if (currentType == baseType)
                {
                    return true;
                }
                var interfaces = new List<Type>(currentType?.GetInterfaces() ?? new Type[] { });
                if (baseType.IsInterface && interfaces.Contains(baseType))
                {
                    return true;
                }
            }
            return false;
        }
    }
}