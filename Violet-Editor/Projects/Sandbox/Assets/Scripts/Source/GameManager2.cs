using System;
using Violet;
namespace Demo
{
	public class GameManager2 : Entity
	{
		Entity[] wood = new Entity[3];
		void OnCreate()
		{
			wood[0] = FindEntityByName("wood1");
			wood[1] = FindEntityByName("wood2");
			wood[2] = FindEntityByName("wood3");

		}

		void OnUpdate(float ts)
		{
			Random random = new Random();

			//entity.GetComponent<Rigidbody2DComponent>().ApplyLinearImpulse(new Vector2(-5.0f, 0.0f), true);
			foreach (var entity in wood)
			{
				entity.GetComponent<Rigidbody2DComponent>().LinearVelocity = new Vector2(-5.0f, 0.0f);
				if(entity.GetComponent<TransformComponent>().Translation.X < -8.5f)
				{
					//Console.Write(1111);

					// 生成随机的正负号
					int sign = random.Next(0, 2) == 0 ? -1 : 1;
					// 在 3 到 6 范围内生成随机数
					float randomNumber = (float)random.NextDouble() * (6 - 3) + 3;
					// 乘以正负号，得到最终的随机数
					randomNumber *= sign;
					entity.GetComponent<Rigidbody2DComponent>().SetPos(new Vector2(7.5f, randomNumber)); 
					//Console.WriteLine(randomNumber);
				}
			}
		}
	}
}
