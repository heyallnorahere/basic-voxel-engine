using BasicVoxelEngine.Components;
using System;

namespace BasicVoxelEngine.Content.Scripts
{
    public sealed class Player : Script
    {
        public override void OnAttach()
        {
            mCameraSensitivity = 0.1f;
            GetComponent<TransformComponent>().Translation = new Vector3(5f, 64f, 5f);
            AddComponent<CameraComponent>().Direction = new Vector3(-1f);
        }
        public override void Update()
        {
            InputManager inputManager = Application.InputManager;
            UpdateCameraDirection(inputManager);
            TakeInput(inputManager);
            // todo: some sort of debug menu
        }
        private void UpdateCameraDirection(InputManager inputManager)
        {
            Vector2 offset = inputManager.Mouse * mCameraSensitivity;
            var camera = GetComponent<CameraComponent>();
            var direction = camera.Direction;
            double x = Math.Asin(direction.Y);
            double factor = Math.Cos(x);
            double y = Math.Atan2(direction.Z / factor, direction.X / factor);
            var radians = new Vector2
            {
                X = (float)x,
                Y = (float)y
            };
            var angle = radians.Degrees;
            angle += new Vector2(offset.Y, offset.X);
            if (angle.X > 89f)
            {
                angle.X = 89f;
            }
            if (angle.X < -89f)
            {
                angle.X = -89f;
            }
            radians = angle.Radians;
            Vector3 newDirection = new Vector3
            {
                X = (float)(Math.Cos(radians.X) * Math.Cos(radians.Y)),
                Y = (float)Math.Sin(radians.X),
                Z = (float)(Math.Cos(radians.X) * Math.Sin(radians.Y))
            };
            camera.Direction = newDirection.Normalized;
        }
        private void TakeInput(InputManager inputManager)
        {
            var camera = GetComponent<CameraComponent>();
            var transform = GetComponent<TransformComponent>();
            float playerSpeed = 2.5f * (float)Application.GetDeltaTime();
            Vector3 forward = camera.Direction * playerSpeed;
            Vector3 up = camera.Up * playerSpeed;
            Vector3 right = camera.Direction.Cross(camera.Up).Normalized * playerSpeed;
            if (inputManager.GetKey(Key.W).Held)
            {
                transform.Translation += forward;
            }
            if (inputManager.GetKey(Key.S).Held)
            {
                transform.Translation -= forward;
            }
            if (inputManager.GetKey(Key.A).Held)
            {
                transform.Translation -= right;
            }
            if (inputManager.GetKey(Key.D).Held)
            {
                transform.Translation += right;
            }
            if (inputManager.GetKey(Key.SPACE).Held)
            {
                transform.Translation += up;
            }
            if (inputManager.GetKey(Key.LEFT_SHIFT).Held)
            {
                transform.Translation -= up;
            }
            if (inputManager.GetKey(Key.Q).Down)
            {
                Application.Quit();
            }
            if (inputManager.GetMouseButton(InputManager.LeftMouseButton).Down)
            {
                LeftClick();
            }
            if (inputManager.GetMouseButton(InputManager.RightMouseButton).Down)
            {
                RightClick();
            }
        }
        private void LeftClick()
        {
            Logger.Print(Logger.Severity.Info, "left clicked");
        }
        private void RightClick()
        {
            Logger.Print(Logger.Severity.Info, "right clicked");
        }
        private float mCameraSensitivity;
    }
}