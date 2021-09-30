using BasicVoxelEngine.Graphics;
using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public static class Application
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Quit();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern double GetDeltaTime();
        public static World World => new World(GetWorld_Native());
        public static InputManager InputManager => new InputManager(GetInputManager_Native());
        public static Factory Factory => new Factory(GetFactory_Native());
        public static Window Window => new Window(GetWindow_Native());
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
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetWorld_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetInputManager_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetFactory_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetWindow_Native();
    }
}