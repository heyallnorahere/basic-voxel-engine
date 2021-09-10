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
        public static void Generate(World world, int seed = -1)
        {
            var builder = new Builder(world, seed);
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
            builder.Execute();
        }
        private Builder(World world, int givenSeed)
        {
            int seed = givenSeed;
            if (seed < 0)
            {
                seed = new Random().Next();
            }
            Seed = seed;
            mWorld = world;
            mStages = new List<List<Step>>();
            for (int i = 0; i < Enum.GetValues(typeof(GenerationStage)).Length; i++)
            {
                mStages.Add(new List<Step>());
            }
            mSetBlocks = new Dictionary<Vector3I, Block>();
        }
        private void AddStep(Step step)
        {
            mStages[(int)step.GenerationStage].Add(step);
        }
        private void Execute()
        {
            Logger.Print(Logger.Severity.Info, "starting world generation");
            var steps = Compile();
            foreach (var step in steps)
            {
                Logger.Print(Logger.Severity.Info, "executing {0}", step.Name ?? "unnamed step");
                step.Callback(this);
            }
        }
        private List<Step> Compile()
        {
            var steps = new List<Step>();
            var registeredTypes = new List<Type>();
            foreach (var stage in mStages)
            {
                foreach (var step in stage)
                {
                    Type type = step.Callback.Method.DeclaringType;
                    if (!registeredTypes.Contains(type))
                    {
                        registeredTypes.Add(type);
                    }
                    steps.Add(step);
                }
            }
            foreach (var type in registeredTypes)
            {
                var methodInfo = type.GetMethod("RearrangeSteps", BindingFlags.Public | BindingFlags.Static, null, new Type[] { typeof(List<Step>) }, null);
                if (methodInfo == null)
                {
                    continue;
                }
                methodInfo.Invoke(null, new object[] { steps });
            }
            return steps;
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
        public int Seed { get; }
        private readonly List<List<Step>> mStages;
        private readonly World mWorld;
        private readonly Dictionary<Vector3I, Block> mSetBlocks;
    }
}
