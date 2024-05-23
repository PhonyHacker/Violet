using System;
using Violet;
namespace Demo
{
	public class GrassCreator : Entity
	{
		public float cols = 50;
		public float rows = 50;
		string[] Grasses =
		{
			"Demo/Assets/Textures/map01.png",
			"Demo/Assets/Textures/map02.png",
			"Demo/Assets/Textures/map03.png",
			"Demo/Assets/Textures/map04.png",
			"Demo/Assets/Textures/map05.png",
			"Demo/Assets/Textures/map06.png",
			"Demo/Assets/Textures/map07.png",
			"Demo/Assets/Textures/map08.png"
		};
		void OnCreate()
		{
			Random random = new Random();
			for (int i = 0; i < cols; i++)
			{
				for (int j = 0; j < rows; j++)
				{
					Entity entity = CreateEntity(i + "," + j);
					entity.Translation = new Vector3(i, j, 0.1f);
					entity.AddComponent<SpriteRendererComponent>();
					entity.GetComponent<SpriteRendererComponent>().Texture2D = Grasses[random.Next(0, Grasses.Length)];
				}
			}
		}

		void OnUpdate(float ts) { }
	}
}
