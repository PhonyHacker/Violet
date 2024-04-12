using Violet;
namespace Sandbox
{
    public class EntityCreator: Entity
    {
        void OnCreate()
        {
            Entity entity = CreateEntity("test");
        }
    }
}
