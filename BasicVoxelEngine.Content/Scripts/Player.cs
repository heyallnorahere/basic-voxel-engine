using BasicVoxelEngine.Components;

namespace BasicVoxelEngine.Content.Scripts
{
    public sealed class Player : Script
    {
        public override void OnAttach()
        {
            GetComponent<TransformComponent>().Translation = new Vector3(5f, 64f, 5f);
            AddComponent<CameraComponent>().Direction = new Vector3(-1f);
        }
        public override void Update()
        {
            UpdateCameraDirection();
            TakeInput();
        }
        private void UpdateCameraDirection()
        {
            // todo: trig
        }
        private void TakeInput()
        {
            // todo: input manager
        }
    }
}