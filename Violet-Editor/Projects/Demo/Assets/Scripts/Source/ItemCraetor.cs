using System;
using Violet;
namespace Demo
{
	public class ItemCraetor : Entity
	{
		string[] Items =
		{
			"Demo/Assets/Textures/Item1.png",
			"Demo/Assets/Textures/Item2.png",
			"Demo/Assets/Textures/Item3.png",
			"Demo/Assets/Textures/Item4.png",
			"Demo/Assets/Textures/Item5.png",
			"Demo/Assets/Textures/Item6.png",
			"Demo/Assets/Textures/Item7.png",
			"Demo/Assets/Textures/Item8.png",
			"Demo/Assets/Textures/Item9.png"
		};
		void OnCreate()
		{
			Random random = new Random();

			int cols = 30;
			int rows = 30;
			for (int i = 0; i < cols; i++)
			{
				for(int j = 0; j < rows; j++) 
				{
					int index = random.Next(0, Items.Length + 8);
					if(index < Items.Length)
					{
						Entity entity = CreateEntity(i + "," + j);
						entity.Translation = new Vector3(i, j, 0.1f);

						entity.AddComponent<SpriteRendererComponent>();
						entity.GetComponent<SpriteRendererComponent>().Texture2D = Items[index];
					}

				}
			}
		}

		void OnUpdate(float ts){}
	}
}
