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

			float posX = GetComponent<TransformComponent>().Translation.X;
			float posY = GetComponent<TransformComponent>().Translation.Y;
			if(posY > 5.5f || posY < -5.5f || posX < -8.0f || GetComponent<Rigidbody2DComponent>().IsContacted())
			{
				GetComponent<Rigidbody2DComponent>().LinearVelocity=new Vector2(-5.0f, 0);
				Console.WriteLine("Loss");
			}

		}
	}
}
