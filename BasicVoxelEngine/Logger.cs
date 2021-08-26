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
        public static void Print(Severity severity, string message, params object[] args)
        {
            string toPrint = string.Format(message, args);
            switch (severity)
            {
            case Severity.Debug:
                PrintDebug(toPrint);
                break;
            case Severity.Info:
                PrintInfo(toPrint);
                break;
            case Severity.Warning:
                PrintWarning(toPrint);
                break;
            case Severity.Error:
                PrintError(toPrint);
                break;
            default:
                throw new ArgumentException("Invalid severity!");
            }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintDebug(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintInfo(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintWarning(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void PrintError(string message);
    }
}