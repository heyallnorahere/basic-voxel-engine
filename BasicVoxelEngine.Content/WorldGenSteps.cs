using BasicVoxelEngine.Content.Blocks;
using BasicVoxelEngine.WorldGen;
using System;
using System.Collections.Generic;

namespace BasicVoxelEngine.Content
{
    public static class WorldGenSteps
    {
        [GenerationStep(nameof(BasicWorldShape), GenerationStage = GenerationStage.World)]
        public static void BasicWorldShape(Builder builder)
        {
            var chunkSize = new Vector3I(128, 256, 128);
            var heightMap = new List<float>(chunkSize.X * chunkSize.Z); // a chunk
            var noise = new FastNoiseLite(builder.Seed);
            noise.SetNoiseType(FastNoiseLite.NoiseType.OpenSimplex2S);
            for (int z = 0; z < chunkSize.Z; z++)
            {
                for (int x = 0; x < chunkSize.X; x++)
                {
                    float value = noise.GetNoise(x, z);
                    value = (value + 1f) / 2f;
                    value /= 5f;
                    heightMap.Add(value);
                }
            }
            var blockRegister = Registry.GetRegister<Block>();
            int grassBlock = blockRegister.GetIndex<Grass>();
            int lightBlock = blockRegister.GetIndex<TestLight>();
            for (int x = 0; x < chunkSize.X; x++)
            {
                for (int z = 0; z < chunkSize.Z; z++)
                {
                    float height = heightMap[(z * chunkSize.X) + x];
                    int y = (int)Math.Floor(height * chunkSize.Y);
                    var position = new Vector3I(x, y, z);
                    builder.SetBlock(position, grassBlock);
                    if ((position * new Vector3I(1, 0, 1)).Length % 20f < 0.001f)
                    {
                        builder.SetBlock(position + new Vector3I(0, 2, 0), lightBlock);
                    }
                }
            }
        }
    }
}
