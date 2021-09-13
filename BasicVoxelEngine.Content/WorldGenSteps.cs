using BasicVoxelEngine.WorldGen;
using System;
using System.Collections.Generic;

namespace BasicVoxelEngine.Content.WorldGen
{
    public static class WorldGenSteps
    {
        [GenerationStep(nameof(BasicWorldShape), GenerationStage = GenerationStage.World)]
        public static void BasicWorldShape(Builder builder)
        {
            var chunkSize = new Vector3I(64, 256, 64);
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
            Block testBlock = blockRegister["bve:grass_block"];
            for (int x = 0; x < chunkSize.X; x++)
            {
                for (int z = 0; z < chunkSize.Z; z++)
                {
                    float height = heightMap[(z * chunkSize.X) + x];
                    int y = (int)Math.Floor(height * chunkSize.Y);
                    var position = new Vector3I(x, y, z);
                    builder.SetBlock(position, testBlock);
                }
            }
        }
        [GenerationStep(nameof(AddLights), GenerationStage = GenerationStage.Structures)]
        public static void AddLights(Builder builder)
        {
            var chunkSize = new Vector3I(16, 256, 16);
            var positions = new Vector3I[]
            {
                new Vector3I(0, chunkSize.Y - 1, 0),
                new Vector3I(chunkSize.X - 1, chunkSize.Y - 1, 0),
                new Vector3I(chunkSize.X - 1, chunkSize.Y - 1, chunkSize.Z - 1),
                new Vector3I(0, chunkSize.Y - 1, chunkSize.Z - 1)
            };
            var blockRegister = Registry.GetRegister<Block>();
            Block testBlock2 = blockRegister["bve:test_block_2"];
            foreach (var position in positions)
            {
                builder.SetBlock(position, testBlock2);
            }
        }
    }
}
