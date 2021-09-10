using System;
using System.Collections.Generic;
using System.Reflection;

namespace BasicVoxelEngine.WorldGen
{
    public sealed class Builder
    {
        private struct Step
        {
            public Action<Builder> Callback { get; set; }
            public string? Name { get; set; }
            public GenerationStage GenerationStage { get; set; }
        }
        private static bool CheckParameters(MethodInfo method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            if (parameters.Length != 1)
            {
                return false;
            }
            return parameters[0].ParameterType == typeof(Builder);
        }
        public static void Generate(World world)
        {
            var builder = new Builder(world);
            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (var assembly in assemblies)
            {
                Type[] types = assembly.GetTypes();
                foreach (var type in types)
                {
                    MethodInfo[] methods = type.GetMethods();
                    foreach (var method in methods)
                    {
                        if (!method.IsStatic)
                        {
                            continue;
                        }
                        var attribute = method.GetCustomAttribute<GenerationStepAttribute>();
                        if (attribute == null)
                        {
                            continue;
                        }
                        else
                        {
                            if (!CheckParameters(method))
                            {
                                throw new ArgumentException("The generation step must have a single argument of type Builder!");
                            }
                            else
                            {
                                var callback = (Action<Builder>)Delegate.CreateDelegate(typeof(Action<Builder>), method);
                                builder.AddStep(new Step
                                {
                                    Callback = callback,
                                    Name = attribute.Name,
                                    GenerationStage = attribute.GenerationStage
                                });
                            }
                        }
                    }
                }
            }
        }
        private Builder(World world)
        {
            mWorld = world;
            mSteps = new List<Step>();
            mSetBlocks = new Dictionary<Vector3I, Block>();
        }
        private void AddStep(Step step)
        {
            mSteps.Add(step);
        }
        public void SetBlock(Vector3I position, Block block)
        {
            mWorld.SetBlock(position, block);
            mSetBlocks[position] = block;
        }
        public void SetBlock(Vector3I position, int index)
        {
            mWorld.SetBlock(position, index);
            mSetBlocks[position] = Registry.GetRegister<Block>()[index];
        }
        public void SetBlock(Vector3I position, NamespacedName namespacedName)
        {
            mWorld.SetBlock(position, namespacedName);
            mSetBlocks[position] = Registry.GetRegister<Block>()[namespacedName];
        }
        public IReadOnlyDictionary<Vector3I, Block> SetBlocks => mSetBlocks;
        private readonly List<Step> mSteps;
        private readonly World mWorld;
        private readonly Dictionary<Vector3I, Block> mSetBlocks;
    }
}
