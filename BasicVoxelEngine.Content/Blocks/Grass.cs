using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister("bve:grass_block")]
    public class Grass : Block
    {
        public override string FriendlyName => nameof(Grass);
    }
}
