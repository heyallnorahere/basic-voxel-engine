﻿namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister("bve:air", PreferredIndex = 0)]
    public class Air : Block
    {
        public override string FriendlyName => nameof(Air);
        public override float Opacity => 0f;
        public override bool Solid => false;
    }
}
