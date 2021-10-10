using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;
using System;

namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister("bve:test_light")]
    public class TestLight : Block
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
        public override string FriendlyName => "Test Light";
        public override Light? Light => mLight;
        private Light? mLight;
    }
}
