using Violet;
namespace Sandbox
{
    public class EntityCreator: Entity
    {
        void OnCreate()
        {
            if(AddComponent<SpriteRendererComponent>())
            {
                // GetComponent<SpriteRendererComponent>().Texture2D = "Resources/Icons/PlayButton.png";
            }
            Entity entity = CreateEntity("test");
        }
    }
}
