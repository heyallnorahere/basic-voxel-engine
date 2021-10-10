using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;
using System;

namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister("bve:test_block", PreferredIndex = 1)]
    public class TestBlock1 : Block
    {
        public override string FriendlyName => "Test block 1";
    }
    [AutoRegister("bve:test_block_2", PreferredIndex = 2)]
    public class TestBlock2 : Block
    {
        protected override void Load(Factory factory, NamespacedName namespacedName)
        {
            mLight = new PointLight
            {
                AmbientStrength = 0.01f,
                SpecularStrength = 0.5f,
                Color = new Vector3(1f),
                Constant = 1f,
                Linear = 0.09f,
                Quadratic = 0.032f
            };
        }
        public override string FriendlyName => "Test block 2";
        public override Light? Light => mLight;
        private Light? mLight;
    }
}
