using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public sealed class Entity
    {
        public Entity()
        {
            ID = 0;
            mWorld = null;
        }
        public Entity(uint id, World world)
        {
            ID = id;
            mWorld = world;
        }
        public uint ID { get; }
        public World World => mWorld ?? throw new NullReferenceException();
        private readonly World? mWorld;
        // todo: figure out how to add an AddComponent<T> method
        public T GetComponent<T>() // todo: add constraint
        {
            return (T)GetComponent_Native(ID, World.mAddress, typeof(T));
        }
        public bool HasComponent<T>()
        {
            return HasComponent_Native(ID, World.mAddress, typeof(T));
        }
        public static implicit operator bool(Entity entity)
        {
            return entity.mWorld != null;
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object GetComponent_Native(uint id, IntPtr world, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(uint id, IntPtr world, Type type);
    }
}