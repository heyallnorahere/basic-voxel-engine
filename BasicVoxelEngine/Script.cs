namespace BasicVoxelEngine
{
    public abstract class Script
    {
        public virtual void OnAttach() { }
        public abstract void Update();
        public Entity Entity { get; set; } = new Entity();
        protected T AddComponent<T>()
        {
            return Entity.AddComponent<T>();
        }
        protected T GetComponent<T>()
        {
            return Entity.GetComponent<T>();
        }
        protected bool HasComponent<T>()
        {
            return Entity.HasComponent<T>();
        }
        protected void RemoveComponent<T>()
        {
            Entity.RemoveComponent<T>();
        }
    }
}