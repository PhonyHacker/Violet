using System;
using Violet;
namespace Demo
{
	public class Shoot : Entity
	{
		public Vector3 velocity = Vector3.Zero;
		public float speed = 1.0f;

		private Vector2 clickPos = Vector2.Zero;
		private Vector2 releasePos = Vector2.Zero;
		private bool isMouseDown = false;

		private Rigidbody2DComponent rigidbody;

		void OnCreate()
		{
			rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void OnUpdate(float ts)
		{
			if(Input.IsMouseButtonDown(MouseButtonCode.ButtonLeft)&&!isMouseDown)
			{
				clickPos = Input.GetMousePosition();
				isMouseDown = true;
			}
			if (!Input.IsMouseButtonDown(MouseButtonCode.ButtonLeft) && isMouseDown)
			{
				releasePos = Input.GetMousePosition();

				Vector2 delta = clickPos - releasePos;
				delta.Y = delta.Y * -1.0f;
				//Console.WriteLine(delta.X + ", " + delta.Y);
				rigidbody.ApplyLinearImpulse(delta * speed * 0.10f, true);

				isMouseDown = false;
			}
		}
	}
}
