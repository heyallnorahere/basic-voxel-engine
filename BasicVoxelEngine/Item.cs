using BasicVoxelEngine.Graphics;
using System;

namespace BasicVoxelEngine
{
    public class Item : RegisteredObject<Item>
    {
        public static class Actions
        {
            public static void Punch(Entity user)
            {
                // todo: animation
                // todo: damage blocks or entities
            }
        }
        /// <summary>
        /// The model that represents this item.
        /// </summary>
        public virtual Model? Model => null;
        /// <summary>
        /// The texture (image) that represents this item
        /// </summary>
        public virtual Texture? Image => null;
        /// <summary>
        /// The maximum quantity of items in an <see cref="ItemStack"/>
        /// </summary>
        public virtual int MaxStack => 64;
        /// <summary>
        /// The function that is called when the player left-clicks
        /// </summary>
        public virtual Action<Entity> LeftClick => Actions.Punch;
    }
}
