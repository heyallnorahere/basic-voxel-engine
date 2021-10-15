using System;
using System.Collections.Generic;

namespace BasicVoxelEngine
{
    /// <summary>
    /// This class is for helpers and extensions.
    /// </summary>
    public static class Helpers
    {
        public static Type GetGenericType(Type baseType, Type[] @params)
        {
            return baseType.MakeGenericType(@params);
        }
        internal static object? GetRegister(Type type)
        {
            var baseMethod = typeof(Registry).GetMethod("GetRegister");
            var getRegister = baseMethod?.MakeGenericMethod(new Type[] { type });
            return getRegister?.Invoke(null, null);
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
        public static float ToRadians(this float value)
        {
            return value / (float)Factor;
        }
        public static float ToDegrees(this float value)
        {
            return value * (float)Factor;
        }
        public static double ToRadians(this double value)
        {
            return value / Factor;
        }
        public static double ToDegrees(this double value)
        {
            return value * Factor;
        }
        private const double Factor = 180D / Math.PI;
    }
}