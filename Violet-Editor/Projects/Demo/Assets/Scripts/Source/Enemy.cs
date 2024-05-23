using System;
using Violet;
namespace Demo
{
	public class Enemy : Entity
	{
		Rigidbody2DComponent rigidbody;
		float time = 0;
		void OnCreate()
		{
			rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void OnUpdate(float ts)
		{
			if (time < 1.0f)
				time += ts;
			else
			{
				if (rigidbody.LinearVelocity.LengthSquared() >= 8.0f)
				{
					DeleteEntity(ID);
					//Console.WriteLine(ID);
				}
			}
		}
	}
}
