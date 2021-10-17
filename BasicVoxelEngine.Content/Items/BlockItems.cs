namespace BasicVoxelEngine.Content.Items
{
    [AutoRegister(Blocks.Dirt.RegisterName)]
    public sealed class Dirt : BlockItem
    {
        public Dirt() : base(typeof(Blocks.Dirt)) { }
    }
    [AutoRegister(Blocks.Grass.RegisterName)]
    public sealed class Grass : BlockItem
    {
        public Grass() : base(typeof(Blocks.Grass)) { }
    }
    [AutoRegister(Blocks.TestLight.RegisterName)]
    public sealed class TestLight : BlockItem
    {
        public TestLight() : base(typeof(Blocks.TestLight)) { }
    }
}
