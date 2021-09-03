using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public static class Logger
    {
        public enum Severity
        {
            Debug,
            Info,
            Warning,
            Error
        }
        public static void Print(Severity severity, string message, params object?[] args)
        {
            string toPrint = string.Format(message, args);
            switch (severity)
            {
            case Severity.Debug:
                PrintDebug_Native(toPrint);
                break;
            case Severity.Info:
                PrintInfo_Native(toPrint);
                break;
            case Severity.Warning:
                PrintWarning_Native(toPrint);
                break;
            case Severity.Error:
                PrintError_Native(toPrint);
                break;
            default:
                throw new ArgumentException("Invalid severity!");
            }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintDebug_Native(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintInfo_Native(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintWarning_Native(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintError_Native(string message);
    }
}