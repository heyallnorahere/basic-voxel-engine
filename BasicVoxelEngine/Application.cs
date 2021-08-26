using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public static class Application
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern double GetDeltaTime();
        public static void TestMethod()
        {
            Logger.Print(Logger.Severity.Info, "Frame time: {0}", GetDeltaTime());
        }
    }
}