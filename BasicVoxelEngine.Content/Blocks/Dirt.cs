using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister("bve:dirt_block")]
    public class Dirt : Block
    {
        public override string FriendlyName => nameof(Dirt);
    }
}
