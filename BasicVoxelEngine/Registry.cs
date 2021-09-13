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
        public IntPtr? NativeAddress { get; internal set; }
        ~RegisteredObject()
        {
            if (NativeAddress != null)
            {
                DestroyRef_Native(NativeAddress ?? throw new NullReferenceException());
            }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr nativeAddress);
        internal void SetIndex(int index)
        {
            mIndex = index;
        }
        public int? Index => mIndex;
        private int? mIndex;
        public override bool Equals(object? obj)
        {
            if (obj is RegisteredObject<T> registeredObject)
            {
                if (NativeAddress != null)
                {
                    if (registeredObject.NativeAddress != null)
                    {
                        return NativeAddress == registeredObject.NativeAddress;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            return base.Equals(obj);
        }
        public override int GetHashCode()
        {
            if (NativeAddress != null)
            {
                return NativeAddress.GetHashCode();
            }
            else
            {
                return base.GetHashCode();
            }
        }
    }
    public sealed class Register<T> : RegisteredObject<Register<T>>, IReadOnlyList<T> where T : RegisteredObject<T>, new()
    {
        private struct Enumerator : IEnumerator<T>
        {
            public Enumerator(Register<T> register)
            {
                mRegistry = register;
                mCurrentIndex = -1;
            }
            public T Current => mRegistry[mCurrentIndex];
            object IEnumerator.Current => Current;
            public void Dispose()
            {
                GC.SuppressFinalize(this);
            }
            public bool MoveNext()
            {
                if (mCurrentIndex < mRegistry.Count - 1)
                {
                    mCurrentIndex++;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            public void Reset()
            {
                mCurrentIndex = -1;
            }
            private readonly Register<T> mRegistry;
            private int mCurrentIndex;
        }
        internal Register(IntPtr nativeAddress)
        {
            base.NativeAddress = nativeAddress;
        }
        public T this[int index]
        {
            get
            {
                if (index > Count)
                {
                    throw new IndexOutOfRangeException();
                }
                if (IsManaged_Native(index, typeof(T), NativeAddress))
                {
                    return GetManagedObject_Native(index, typeof(T), NativeAddress);
                }
                else
                {
                    var @object = new T();
                    @object.NativeAddress = CreateRef_Native(index, typeof(T), NativeAddress);
                    return @object;
                }
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
            int index = RegisterObject_Native(@object, namespacedName, typeof(T), NativeAddress);
            @object.SetIndex(index);
            return index;
        }
        public int? GetIndex(NamespacedName namespacedName)
        {
            int index;
            if (GetIndex_Native(namespacedName, typeof(T), NativeAddress, out index))
            {
                return index;
            }
            else
            {
                return null;
            }
        }
        public NamespacedName? GetNamespacedName(int index)
        {
            NamespacedName namespacedName;
            if (GetNamespacedName_Native(index, typeof(T), NativeAddress, out namespacedName))
            {
                return namespacedName;
            }
            else
            {
                return null;
            }
        }
        public IEnumerator<T> GetEnumerator()
        {
            return new Enumerator(this);
        }
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        public int Count => GetCount_Native(typeof(T), NativeAddress);
        private new IntPtr NativeAddress => base.NativeAddress ?? throw new NullReferenceException();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr CreateRef_Native(int index, Type type, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetCount_Native(Type type, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool GetIndex_Native(NamespacedName namespacedName, Type type, IntPtr address, out int index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool GetNamespacedName_Native(int index, Type type, IntPtr address, out NamespacedName namespacedName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int RegisterObject_Native(T @object, NamespacedName namespacedName, Type type, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsManaged_Native(int index, Type type, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern T GetManagedObject_Native(int index, Type type, IntPtr address);
    }
    public static class Registry
    {
        static Registry()
        {
            RegisterTypes_Native();
        }
        public static bool RegisterExists<T>() where T : RegisteredObject<T>, new()
        {
            return RegisterExists_Native(typeof(T));
        }
        public static Register<T> GetRegister<T>() where T : RegisteredObject<T>, new()
        {
            if (!RegisterExists<T>())
            {
                throw new ArgumentException("The specified register does not exist!");
            }
            IntPtr address = CreateRegisterRef_Native(typeof(T));
            return new Register<T>(address);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void RegisterTypes_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool RegisterExists_Native(Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr CreateRegisterRef_Native(Type type);
    }
}