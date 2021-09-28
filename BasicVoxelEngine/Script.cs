namespace BasicVoxelEngine
{
    public abstract class Script
    {
        public abstract void Update();
        public Entity Entity { get; set; } = new Entity();
    }
}