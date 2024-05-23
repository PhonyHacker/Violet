using Violet;
namespace Demo { 

	public class Player : Entity
	{
		public float Speed = 3.0f;
		public float time = 3.0f;

		private Rigidbody2DComponent m_Rigidbody;

		void OnCreate()
		{
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void OnUpdate(float ts)
		{
			if (time < 3) time += ts; 
			Vector3 velocity = Vector3.Zero;

			if (m_Rigidbody.IsContacted())
			{
				if (Input.IsKeyDown(KeyCode.Space))
				{
					if (time >= 1.8)
					{
						velocity.Y = 32.0f;
						time = 0.0f;
					}
				}
			}

			//if (Input.IsKeyDown(KeyCode.W))
				//velocity.Y = 0.1f;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity.X = 1.0f;
			velocity *= Speed * ts;

			m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);

			Entity camera = FindEntityByName("CameraA");
			Vector3 pos = GetComponent<TransformComponent>().Translation;
			camera.GetComponent<TransformComponent>().Translation = new Vector3(pos.X, pos.Y, 5.1f);

		}

	}
}
