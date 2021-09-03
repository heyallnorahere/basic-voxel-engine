using System;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public static class Application
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern double GetDeltaTime();
        public static void TestMethod()
        {
            Register<Block> blockRegister = Registry.GetRegister<Block>();
            for (int i = 0; i < blockRegister.Count; i++)
            {
                NamespacedName namespacedName = blockRegister.GetNamespacedName(i) ?? throw new NullReferenceException();
                Block block = blockRegister[i];
                Logger.Print(Logger.Severity.Info, $"Block: {namespacedName} ({i}): {block.FriendlyName}");
            }
        }
        internal static void LoadContent()
        {
            // a whole lot of reflection. not great, i know.
            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (var assembly in assemblies)
            {
                Type[] types = assembly.GetTypes();
                foreach (var type in types)
                {
                    var attribute = type.GetCustomAttribute<AutoRegisterAttribute>();
                    Type? derivedBase;
                    if (attribute != null && IsDerived(typeof(RegisteredObject<>), type, out derivedBase))
                    {
                        if (derivedBase != null)
                        {
                            var getRegister = typeof(Registry).GetMethod("GetRegister")?.MakeGenericMethod(new Type[] { derivedBase })
                                ?? throw new NullReferenceException();
                            var registerType = Helpers.GetGenericType(typeof(Register<>), new Type[] { derivedBase });
                            var registerMethod = registerType.GetMethod("RegisterObject", new Type[] { derivedBase, typeof(NamespacedName) })
                                ?? throw new NullReferenceException();
                            var register = getRegister.Invoke(null, null);
                            var constructor = type.GetConstructor(new Type[] { }) ?? throw new NullReferenceException($"Could not find a viable constructor for type: {type}");
                            var instance = constructor.Invoke(null);
                            registerMethod.Invoke(register, new object?[] { instance, attribute.Name });
                        }
                    }
                }
            }
        }
        private static bool IsDerived(Type baseType, Type derivedType, out Type? derivedBase)
        {
            Type? currentType = derivedType;
            while (currentType != null)
            {
                Type fullType;
                if ((currentType.BaseType?.GenericTypeArguments?.Length ?? 0) > 0)
                {
                    fullType = Helpers.GetGenericType(baseType, currentType.BaseType?.GenericTypeArguments ?? throw new NullReferenceException());
                }
                else
                {
                    fullType = baseType;
                }
                if (currentType.BaseType == fullType)
                {
                    derivedBase = currentType;
                    return true;
                }
                currentType = currentType.BaseType;
            }
            derivedBase = null;
            return false;
        }
    }
}