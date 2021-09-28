using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public sealed class Entity
    {
        static Entity()
        {
            RegisterComponents_Native();
        }
        public Entity(uint id, World world)
        {
            ID = id;
            mWorld = world;
        }
        public Entity()
        {
            ID = uint.MaxValue;
            mWorld = null;
        }
        public uint ID { get; }
        public World World => mWorld ?? throw new NullReferenceException();
        private readonly World? mWorld;
        public T AddComponent<T>()
        {
            return (T)AddComponent_Native(ID, World.mAddress, typeof(T));
        }
        public T GetComponent<T>()
        {
            return (T)GetComponent_Native(ID, World.mAddress, typeof(T));
        }
        public bool HasComponent<T>()
        {
            return HasComponent_Native(ID, World.mAddress, typeof(T));
        }
        public void RemoveComponent<T>()
        {
            RemoveComponent_Native(ID, World.mAddress, typeof(T));
        }
        public static implicit operator bool(Entity entity)
        {
            return entity.mWorld != null && entity.ID != uint.MaxValue;
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object AddComponent_Native(uint id, IntPtr world, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object GetComponent_Native(uint id, IntPtr world, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(uint id, IntPtr world, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void RemoveComponent_Native(uint id, IntPtr world, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void RegisterComponents_Native();
    }
}