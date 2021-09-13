using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public static class Application
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern double GetDeltaTime();
        public static World World => new World(GetWorld_Native());
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
            var autoRegisteredObjects = new Dictionary<Type, Dictionary<object, AutoRegisterAttribute>>(); 
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
                            var constructor = type.GetConstructor(new Type[] { }) ?? throw new NullReferenceException($"Could not find a viable constructor for type: {type}");
                            object instance = constructor.Invoke(null);
                            if (!autoRegisteredObjects.ContainsKey(derivedBase))
                            {
                                autoRegisteredObjects.Add(derivedBase, new Dictionary<object, AutoRegisterAttribute>());
                            }
                            autoRegisteredObjects[derivedBase].Add(instance, attribute);
                        }
                    }
                }
            }
            foreach (var pair in autoRegisteredObjects)
            {
                var getRegister = typeof(Registry).GetMethod("GetRegister")?.MakeGenericMethod(new Type[] { pair.Key })
                    ?? throw new NullReferenceException();
                var registerType = Helpers.GetGenericType(typeof(Register<>), new Type[] { pair.Key });
                var registerMethod = registerType.GetMethod("RegisterObject", new Type[] { pair.Key, typeof(NamespacedName) })
                    ?? throw new NullReferenceException();
                var register = getRegister.Invoke(null, null);
                var countProperty = registerType.GetProperty("Count");
                int count = (int)countProperty.GetValue(register);
                var toRegister = new List<KeyValuePair<object, NamespacedName>?>();
                foreach (var @object in pair.Value)
                {
                    var toAdd = new KeyValuePair<object, NamespacedName>(@object.Key, @object.Value.Name);
                    int index = @object.Value.PreferredIndex;
                    if (index < 0)
                    {
                        index = 99; // a good starting point
                    }
                    if (index < count + toRegister.Count)
                    {
                        if (toRegister[index - count] != null)
                        {
                            int insertionIndex = FindFirstAvailableIndex(toRegister, index);
                            if (insertionIndex != -1)
                            {
                                toRegister[insertionIndex] = toAdd;
                            }
                            else
                            {
                                toRegister.Add(toAdd);
                            }
                        }
                        else
                        {
                            toRegister[index] = toAdd;
                        }
                    }
                    else
                    {
                        for (int i = count + toRegister.Count; i < index; i++)
                        {
                            toRegister.Add(null);
                        }
                        toRegister.Add(toAdd);
                    }
                }
                foreach (var pairToRegister in toRegister)
                {
                    if (pairToRegister == null)
                    {
                        continue;
                    }
                    var verifiedPair = (KeyValuePair<object, NamespacedName>)pairToRegister;
                    registerMethod.Invoke(register, new object[] { verifiedPair.Key, verifiedPair.Value });
                }
            }
        }
        private static int FindFirstAvailableIndex<T>(IReadOnlyList<T?> list, int preferred = 0) where T : struct
        {
            // to avoid a very large list
            for (int i = preferred; i >= 0; i--)
            {
                if (list[i] == null)
                {
                    return i;
                }
            }
            return -1;
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
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetWorld_Native();
    }
}