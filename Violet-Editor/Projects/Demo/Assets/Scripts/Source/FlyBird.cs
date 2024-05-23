using System;
using Violet;
namespace Demo
{
	public class FlyBird : Entity
	{
		private float time = 1.0f;
		void OnCreate()
		{
			//if(GetComponent<Rigidbody2DComponent>() != null)
			//{
			//	var body = GetComponent<Rigidbody2DComponent>();
			//	body.ApplyLinearImpulse(new Vector2(0.0f, 5f), true);
			//	Console.WriteLine("Init");

			//}
		}
		
		void OnUpdate(float ts)
		{
			if (time < 2) time += ts;
			if (Input.IsMouseButtonDown(MouseButtonCode.ButtonLeft) && GetComponent<Rigidbody2DComponent>()!=null &&time >0.2f)
			{
				time = 0;
				var body = GetComponent<Rigidbody2DComponent>();
				body.ApplyLinearImpulse(new Vector2(0.0f, 3f), true);
			}

			if (Input.IsKeyDown(KeyCode.Q))
				Game.Stop();
				



		}
	}
}
