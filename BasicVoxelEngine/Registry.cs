using BasicVoxelEngine.Graphics;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
    public sealed class AutoRegisterAttribute : Attribute
    {
        public AutoRegisterAttribute(string fullName)
        {
            Name = fullName;
        }
        public AutoRegisterAttribute(string namespaceName, string localName)
        {
            Name = new NamespacedName(namespaceName, localName);
        }
        public NamespacedName Name { get; }
        public int PreferredIndex
        {
            get => mPreferredIndex ?? -1;
            set => mPreferredIndex = value;
        }
        internal int? mPreferredIndex = null;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct NamespacedName
    {
        public const char Separator = ':';
        public NamespacedName(string fullName)
        {
            string namespaceName, localName;
            Convert(fullName, out namespaceName, out localName);
            NamespaceName = namespaceName;
            LocalName = localName;
        }
        public NamespacedName(string namespaceName, string localName)
        {
            NamespaceName = namespaceName;
            LocalName = localName;
        }
        public string NamespaceName { get; set; }
        public string LocalName { get; set; }
        public string FullName => NamespaceName + Separator + LocalName;
        public override string ToString() => FullName;
        public static implicit operator NamespacedName(string fullName) => new NamespacedName(fullName);
        public static implicit operator string(NamespacedName namespacedName) => namespacedName.FullName;
        private static void Convert(string name, out string namespaceName, out string localName)
        {
            string[] sections = name.Split(Separator);
            if (sections.Length > 1)
            {
                namespaceName = sections[0];
                var followingSections = new List<string>();
                for (int i = 1; i < sections.Length; i++)
                {
                    followingSections.Add(sections[i]);
                }
                localName = followingSections[0];
                for (int i = 1; i < followingSections.Count; i++)
                {
                    localName += Separator + followingSections[i];
                }
            }
            else
            {
                namespaceName = string.Empty;
                localName = sections[0];
            }
        }
    }
    public abstract class RegisteredObject<T> where T : RegisteredObject<T>
    {
        /// <summary>
        /// This function is called just before the game runs and after every other registered object is registered
        /// </summary>
        protected virtual void Load(Factory factory, NamespacedName namespacedName) { }
        internal void DoLoad(Factory factory, NamespacedName namespacedName) => Load(factory, namespacedName);
        /// <summary>
        /// The friendly name of this block.
        /// </summary>
        public virtual string FriendlyName => Registry.GetRegister<T>().GetNamespacedName(RegisterIndex)?.FullName ?? (GetType().FullName ?? "Unknown object");
        public int RegisterIndex { get; internal set; } = -1;
    }
    public sealed class Register<T> : IReadOnlyList<T> where T : RegisteredObject<T>
    {
        internal Register()
        {
            mItems = new List<T>();
            mNameMap = new Dictionary<NamespacedName, int>();
            mIdMap = new Dictionary<int, NamespacedName>();
        }
        public T this[int index]
        {
            get
            {
                if (index > Count)
                {
                    throw new IndexOutOfRangeException();
                }
                return mItems[index];
            }
        }
        public T this[NamespacedName name]
        {
            get
            {
                int? index = GetIndex(name);
                if (index == null)
                {
                    throw new ArgumentOutOfRangeException($"No valid index was found for name {name}!");
                }
                else
                {
                    return this[(int)index];
                }
            }
        }
        public int RegisterObject(T @object, NamespacedName namespacedName)
        {
            int index = Count;
            @object.RegisterIndex = index;
            mItems.Add(@object);
            mNameMap.Add(namespacedName, index);
            mIdMap.Add(index, namespacedName);
            return index;
        }
        public bool GetInstance<T2>(out T2? instance) where T2 : class, T
        {
            if (typeof(T2) == typeof(T))
            {
                throw new ArgumentException("Cannot get an instance of the base class!");
            }
            foreach (T element in this)
            {
                if (element is T2 casted)
                {
                    instance = casted;
                    return true;
                }
            }
            instance = null;
            return false;
        }
        public T2 GetInstance<T2>() where T2 : class, T
        {
            T2? instance;
            if (GetInstance(out instance))
            {
                return instance ?? throw new NullReferenceException();
            }
            else
            {
                throw new ArgumentException("No block of the given type was registered!");
            }
        }
        public int? GetIndex(NamespacedName namespacedName)
        {
            if (mNameMap.ContainsKey(namespacedName))
            {
                return mNameMap[namespacedName];
            }
            else
            {
                return null;
            }
        }
        public int GetIndex<T2>() where T2 : T
        {
            if (typeof(T2) == typeof(T))
            {
                throw new ArgumentException("Cannot determine an index based off of base type!");
            }
            for (int i = 0; i < Count; i++)
            {
                T element = this[i];
                if (element is T2)
                {
                    return i;
                }
            }
            return -1;
        }
        public NamespacedName? GetNamespacedName(int index)
        {
            if (mIdMap.ContainsKey(index))
            {
                return mIdMap[index];
            }
            else
            {
                return null;
            }
        }
        public bool GetNamespacedName<T2>(out NamespacedName? namespacedName) where T2 : T
        {
            int index = GetIndex<T2>();
            if (index != -1)
            {
                namespacedName = GetNamespacedName(index);
                return true;
            }
            else
            {
                namespacedName = null;
                return false;
            }
        }
        public NamespacedName GetNamespacedName<T2>() where T2 : T
        {
            NamespacedName? namespacedName;
            if (GetNamespacedName<T2>(out namespacedName))
            {
                return (NamespacedName)(namespacedName ?? throw new NullReferenceException());
            }
            else
            {
                throw new ArgumentException("Cannot find the name for an unregistered object!");
            }
        }
        public IEnumerator<T> GetEnumerator() => mItems.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        public int Count => mItems.Count;
        private readonly List<T> mItems;
        private readonly Dictionary<NamespacedName, int> mNameMap;
        private readonly Dictionary<int, NamespacedName> mIdMap;
    }
    public static class Registry
    {
        static Registry()
        {
            mRegisters = new Dictionary<Type, object>();
        }
        public static Register<T> GetRegister<T>() where T : RegisteredObject<T>
        {
            if (!mRegisters.ContainsKey(typeof(T)))
            {
                mRegisters.Add(typeof(T), new Register<T>());
            }
            return (Register<T>)mRegisters[typeof(T)];
        }
        public static ICollection<Type> UsedRegisterTypes => mRegisters.Keys;
        private static readonly Dictionary<Type, object> mRegisters;
    }
}