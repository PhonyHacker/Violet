using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Violet;

namespace Sandbox
{
    public class Player : Entity
    {
        public float Speed = 0.5f;
        public float Time = 0.0f;
        // public Vector3 v3;
        public float curSpeedX = 0.0f;
        public float curSpeedY = 0.0f;

        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_Rigidbody;

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");

            m_Transform = GetComponent<TransformComponent>();
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }

        void OnUpdate(float ts)
        {
            if(Input.IsMouseBUttonDown(MouseButtonCode.ButtonLeft))
            {
                // Console.WriteLine("Mouse Pressed:{" + Input.GetMousePosition().X + ", " + Input.GetMousePosition().Y + "}");
            }
            else
            {
                // Console.WriteLine("Mouse Not Pressed But:{" + Input.GetMousePosition().X + ", " + Input.GetMousePosition().Y + "}");
            }

            Time += ts;
            // Console.WriteLine($"Player.OnUpdate: {ts}");

            curSpeedX = m_Rigidbody.LinearVelocity.X;

            float tempY = m_Rigidbody.LinearVelocity.Y;
            if (tempY < float.Epsilon && tempY < -float.Epsilon)
                tempY = 0;
            curSpeedY = tempY;

            float speed = Speed;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 0.1f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -1.0f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 1.0f;

            Entity cameraEntity = FindEntityByName("CameraA");
            if (cameraEntity != null)
            {
                Camera camera = cameraEntity.As<Camera>();

                if (Input.IsKeyDown(KeyCode.Q))
                {
                    //Console.WriteLine("Q From C#");
                    camera.DistanceFromPlayer += speed * 2.0f * ts;
                }
                else if (Input.IsKeyDown(KeyCode.E))
                {
                    camera.DistanceFromPlayer -= speed * 2.0f * ts;
                }
            }

            velocity *= speed * ts;

            m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);
        }

    }
}
