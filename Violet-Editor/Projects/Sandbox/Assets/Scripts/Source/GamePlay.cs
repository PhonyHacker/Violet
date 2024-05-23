using System;
using Violet;
namespace Sandbox
{
	public class GamePlay : Entity
	{
		void OnCreate()
		{
			int col = 100;
			int row = 10;

			for (int i = 0; i < col; i++) 
			{
				for (int j = 0; j < row; j++) 
				{
					Entity entity = CreateEntity(i + "," + j);
					entity = CreateEntity(i + "," + j);
					entity.GetComponent<TransformComponent>().Translation = new Vector3(i, j, 0);
					entity.AddComponent<SpriteRendererComponent>();
					entity.GetComponent<SpriteRendererComponent>().Texture2D = "Sandbox/Assets/Textures/tree1.png";
				}
			}
		}

		void OnUpdate(float ts)
		{

		}
	}
}
