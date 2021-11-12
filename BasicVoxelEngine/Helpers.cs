using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

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
        internal static int SizeOf(Type type) => Marshal.SizeOf(type);
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
        public static bool Implements(this Type type, Type interfaceType)
        {
            if (!interfaceType.IsInterface)
            {
                throw new ArgumentException("The given interface type is not an interface!");
            }
            foreach (Type implementedInterface in type.GetInterfaces())
            {
                if (interfaceType.DerivesFrom(implementedInterface))
                {
                    return true;
                }
            }
            return false;
        }
        public static UniformBuffer CreateUniformBuffer(this Factory factory, out Buffer allocatedData, ShaderReflectionData reflectionData, uint binding, uint set = 0)
        {
            uint bufferBinding = binding % 16;
            uint descriptorSet = set + (binding - bufferBinding) / 16;
            var descriptorSetData = reflectionData.DescriptorSets[descriptorSet];
            var bufferData = descriptorSetData.UniformBuffers[bufferBinding];
            var bufferType = bufferData.Type;
            allocatedData = new Buffer();
            allocatedData.Alloc(bufferType.Size);
            allocatedData.Zero();
            var ubo = factory.CreateUniformBuffer(bufferType.Size, descriptorSet * 16 + bufferBinding);
            ubo.SetData(allocatedData);
            return ubo;
        }
        public static StorageBuffer CreateStorageBuffer(this Factory factory, out Buffer allocatedData, ShaderReflectionData reflectionData, uint binding, uint set = 0)
        {
            uint bufferBinding = binding % 16;
            uint descriptorSet = set + (binding - bufferBinding) / 16;
            var descriptorSetData = reflectionData.DescriptorSets[descriptorSet];
            var bufferData = descriptorSetData.StorageBuffers[bufferBinding];
            var bufferType = bufferData.Type;
            allocatedData = new Buffer();
            allocatedData.Alloc(bufferType.Size);
            allocatedData.Zero();
            var ssbo = factory.CreateStorageBuffer(bufferType.Size, descriptorSet * 16 + bufferBinding);
            ssbo.SetData(allocatedData);
            return ssbo;
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool AreRefsEqual(IntPtr ref1, IntPtr ref2);
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