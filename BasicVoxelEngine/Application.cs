using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    public static class Application
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern double GetDeltaTime();
        public static void TestMethod()
        {
            Logger.Print(Logger.Severity.Info, $"{Registry.GetRegister<Block>().GetIndex("bve:air")}");
        }
    }
}