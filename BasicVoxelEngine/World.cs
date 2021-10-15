using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public sealed class World
    {
        internal World(IntPtr address)
        {
            mAddress = address;
        }
        ~World()
        {
            Destroy_Native(mAddress);
        }
        public Block GetBlock(Vector3I position)
        {
            int index;
            GetBlock(position, out index);
            var blockRegister = Registry.GetRegister<Block>();
            return blockRegister[index];
        }
        public void GetBlock(Vector3I position, out int index)
        {
            index = GetBlock_Native(mAddress, position);
        }
        public void GetBlock(Vector3I position, out NamespacedName namespacedName)
        {
            int index;
            GetBlock(position, out index);
            var blockRegister = Registry.GetRegister<Block>();
            namespacedName = blockRegister.GetNamespacedName(index) ?? throw new ArgumentException($"Block {index} does not have a name!");
        }
        public void SetBlock(Vector3I position, Block block)
        {
            int index = block.RegisterIndex;
            if (index < 0)
            {
                throw new ArgumentException($"{block.FriendlyName} is not registered!");
            }
            SetBlock(position, index);
        }
        public void SetBlock(Vector3I position, int index)
        {
            if (index >= Registry.GetRegister<Block>().Count)
            {
                throw new ArgumentException("This block does not exist!");
            }
            SetBlock_Native(mAddress, position, index);
        }
        public void SetBlock(Vector3I position, NamespacedName namespacedName)
        {
            var blockRegister = Registry.GetRegister<Block>();
            int index = blockRegister.GetIndex(namespacedName)
                ?? throw new ArgumentException($"The block name {namespacedName} does not have an associated index!");
            SetBlock(position, index);
        }
        public void AddOnBlockChangedCallback(Action<Vector3I, World> callback)
        {
            AddOnBlockChangedCallback_Native(mAddress, callback);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetBlock_Native(IntPtr address, Vector3I position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetBlock_Native(IntPtr address, Vector3I position, int index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AddOnBlockChangedCallback_Native(IntPtr address, Delegate callback);
        internal readonly IntPtr mAddress;
    }
}
