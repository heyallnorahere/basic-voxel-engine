﻿using System;

namespace BasicVoxelEngine.WorldGen
{
    public enum GenerationStage
    {
        World,
        Shinies,
        Structures
    }
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public sealed class GenerationStepAttribute : Attribute
    {
        public GenerationStepAttribute()
        {
            Name = null;
        }
        public GenerationStepAttribute(string name)
        {
            Name = name;
        }
        public string? Name { get; }
        public GenerationStage GenerationStage { get; set; } = GenerationStage.World;
    }
}
