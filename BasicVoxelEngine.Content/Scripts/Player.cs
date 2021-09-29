using BasicVoxelEngine.Components;
using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;

namespace BasicVoxelEngine.Content.Scripts
{
    public sealed class Player : Script
    {
        private static void PrintTextureData(string name, Texture texture)
        {
            Logger.Print(Logger.Severity.Info, "Texture: {0}", name);
            Vector2I size = texture.Size;
            Logger.Print(Logger.Severity.Info, "\tWidth: {0}, height: {1}", size.X, size.Y);
            Logger.Print(Logger.Severity.Info, "\tChannels: {0}", texture.Channels);
        }
        public override void OnAttach()
        {
            mCameraSensitivity = 0.1f;
            GetComponent<TransformComponent>().Translation = new Vector3(5f, 64f, 5f);
            AddComponent<CameraComponent>().Direction = new Vector3(-1f);
            Factory factory = Application.Factory;
            Model model = Model.LoadModel(AssetManager.GetAssetPath("model:bve:model_block.obj"), factory);
            var textures = new List<Texture>();
            for (int i = 0; i < model.MeshCount; i++)
            {
                MeshData meshData = model.GetMesh(i);
                textures.AddRange(meshData.Textures);
            }
            Logger.Print(Logger.Severity.Info, "{0} vertices, {1} indices, {2} textures", model.Vertices.Count, model.Indices.Count, textures.Count);
            for (int i = 0; i < textures.Count; i++)
            {
                PrintTextureData($"Model texture {i + 1}", textures[i]);
            }
            Texture loadedTexture = factory.CreateTexture(AssetManager.GetAssetPath("block:bve:grass_block.png"));
            PrintTextureData("Loaded texture", loadedTexture);
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
        }
        private float mCameraSensitivity;
    }
}