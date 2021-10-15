using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace BasicVoxelEngine.ContentLoading
{
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = false)]
    public sealed class AssemblyContentLoaderAttribute : Attribute
    {
        public AssemblyContentLoaderAttribute(Type loaderType)
        {
            if (!loaderType.DerivesFrom(typeof(ContentLoader)))
            {
                throw new ArgumentException("The given type must derive from ContentLoader!");
            }
            ConstructorInfo? constructor = loaderType.GetConstructor(new Type[] { });
            if (constructor == null)
            {
                throw new ArgumentException("The assembly content loader must have a constructor that takes no arguments!");
            }
            Loader = (ContentLoader)constructor.Invoke(new object[] { });
        }
        public ContentLoader Loader { get; }
    }
    public abstract class ContentLoader
    {
        internal struct RegistrationDescriptor
        {
            public object Data { get; set; }
            public int PreferredIndex { get; set; }
            public string RegisterName { get; set; }
        }
        protected sealed class LoaderAgent
        {
            internal LoaderAgent()
            {
                mDictionary = new Dictionary<Type, List<RegistrationDescriptor>>();
            }
            public void Register(object @object, string registerName, int preferredIndex = -1)
            {
                Type? derivedBase;
                if (!IsDerived(typeof(RegisteredObject<>), @object.GetType(), out derivedBase) || derivedBase == null)
                {
                    throw new ArgumentException("The given type must inherit RegisteredObject<>, but not directly!");
                }
                if (!mDictionary.ContainsKey(derivedBase))
                {
                    mDictionary.Add(derivedBase, new List<RegistrationDescriptor>());
                }
                mDictionary[derivedBase].Add(new RegistrationDescriptor
                {
                    Data = @object,
                    PreferredIndex = preferredIndex,
                    RegisterName = registerName
                });
            }
            internal IReadOnlyDictionary<Type, List<RegistrationDescriptor>> Dictionary => mDictionary;
            private readonly Dictionary<Type, List<RegistrationDescriptor>> mDictionary;
        }
        protected virtual void Load(LoaderAgent agent) { }
        protected virtual void OnLoaded() { }
        internal static void LoadContent()
        {
            // a whole lot of reflection. not great, i know.
            var agent = new LoaderAgent();
            var loaders = new List<ContentLoader>();
            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (var assembly in assemblies)
            {
                var loaderAttribute = assembly.GetCustomAttribute<AssemblyContentLoaderAttribute>();
                if (loaderAttribute != null)
                {
                    loaderAttribute.Loader.Load(agent);
                    loaders.Add(loaderAttribute.Loader);
                }
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
                            agent.Register(instance, attribute.Name, attribute.PreferredIndex);
                        }
                    }
                }
            }
            foreach (var pair in agent.Dictionary)
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
                foreach (var descriptor in pair.Value)
                {
                    var toAdd = new KeyValuePair<object, NamespacedName>(descriptor.Data, descriptor.RegisterName);
                    int index = descriptor.PreferredIndex;
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
            foreach (var loader in loaders)
            {
                loader.OnLoaded();
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
        internal static void LoadRegisteredObjectsGeneric<T>(Factory factory) where T : RegisteredObject<T>
        {
            var register = Registry.GetRegister<T>();
            for (int i = 0; i < register.Count; i++)
            {
                var namespacedName = register.GetNamespacedName(i);
                if (namespacedName != null)
                {
                    register[i].DoLoad(factory, (NamespacedName)namespacedName);
                }
            }
        }
        internal static void LoadRegisteredObjects()
        {
            Factory factory = Application.Factory;
            foreach (Type type in Registry.UsedRegisterTypes)
            {
                var loadObjectsBase = typeof(ContentLoader).GetMethod("LoadRegisteredObjectsGeneric", BindingFlags.NonPublic | BindingFlags.Static);
                var loadObjects = loadObjectsBase?.MakeGenericMethod(new Type[] { type });
                loadObjects?.Invoke(null, new object[] { factory });
            }
        }
    }
}