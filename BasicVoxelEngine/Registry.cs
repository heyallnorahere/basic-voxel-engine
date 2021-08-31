using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
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
        public static implicit operator NamespacedName(string fullName) => new(fullName);
        public static implicit operator string(NamespacedName namespacedName) => namespacedName.FullName;
        private static void Convert(string name, out string namespaceName, out string localName)
        {
            string[] sections = name.Split(Separator);
            if (sections.Length > 1)
            {
                namespaceName = sections[0];
                string[] followingSections = sections[1..];
                localName = followingSections[0];
                for (int i = 1; i < followingSections.Length; i++)
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
        public IntPtr? NativeAddress
        {
            get => mNativeAddress;
            internal set
            {
                mNativeAddress = value;
                OnReady?.Invoke();
            }
        }
        ~RegisteredObject()
        {
            if (mNativeAddress != null)
            {
                DestroyRef_Native(NativeAddress ?? throw new NullReferenceException(), typeof(T));
            }
        }
        protected event Action? OnReady;
        private IntPtr? mNativeAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool DestroyRef_Native(IntPtr nativeAddress, Type type);
    }
    public class Register<T> : RegisteredObject<Register<T>> where T : RegisteredObject<T>, new()
    {
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
            return RegisterObject_Native(@object, namespacedName, typeof(T), NativeAddress);
        }
        public int? GetIndex(NamespacedName namespacedName) => GetIndex_Native(namespacedName, typeof(T), NativeAddress);
        public NamespacedName? GetNamespacedName(int index) => GetNamespacedName_Native(index, typeof(T), NativeAddress);
        public int Count => GetCount_Native(typeof(T), NativeAddress);
        private new IntPtr NativeAddress => base.NativeAddress ?? throw new NullReferenceException();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr CreateRef_Native(int index, Type type, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetCount_Native(Type type, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int? GetIndex_Native(NamespacedName namespacedName, Type type, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern NamespacedName? GetNamespacedName_Native(int index, Type type, IntPtr address);
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