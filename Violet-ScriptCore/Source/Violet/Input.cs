namespace Violet
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode keycode)
		{
			return InternalCalls.Input_IsKeyDown(keycode);
		}

		public static bool IsMouseButtonDown(MouseButtonCode button)
		{
			return InternalCalls.Input_MouseButtonPressed(button);
		}

		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 position);
			return position;
		}
        public static Vector2 GetMouseImGuiPosition()
        {
            InternalCalls.Input_GetMouseImGuiPosition(out Vector2 position);
            return position;
        }
		public static Entity GetMouseHoever()
		{
            ulong entityID = InternalCalls.Input_GetMouseHoever();
			if (entityID == 0)
				return null;
            return new Entity(entityID);
        }
    }
}
