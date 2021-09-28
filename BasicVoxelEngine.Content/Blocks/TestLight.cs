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
            mLight = new Spotlight
            {
                AmbientStrength = 0.01f,
                SpecularStrength = 0.5f,
                Color = new Vector3(1f),
                Cutoff = Convert.ToSingle(Math.Cos(30 / (180 / Math.PI))),
                Direction = new Vector3(0f, -1f, 0f)
            };
        }
        public override string FriendlyName => "Test Light";
        public override Light? Light => mLight;
        private Light? mLight;
    }
}
