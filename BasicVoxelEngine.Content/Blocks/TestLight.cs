using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;

namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister(RegisterName)]
    public class TestLight : Block
    {
        public const string RegisterName = "bve:test_light";
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
