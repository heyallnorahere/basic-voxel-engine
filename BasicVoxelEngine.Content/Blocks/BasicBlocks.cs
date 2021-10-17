namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister(RegisterName)]
    public class Dirt : Block
    {
        public const string RegisterName = "bve:dirt_block";
        public override string FriendlyName => nameof(Dirt);
    }
    [AutoRegister(RegisterName)]
    public class Grass : Block
    {
        public const string RegisterName = "bve:grass_block";
        public override string FriendlyName => nameof(Grass);
    }
}
