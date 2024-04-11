namespace Violet
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode keycode)
		{
			return InternalCalls.Input_IsKeyDown(keycode);
		}

		public static bool IsMouseBUttonDown(MouseButtonCode button)
		{
			return InternalCalls.Input_MouseButtonPressed(button);
		}

		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 position);
			return position;
		}
	}
}
