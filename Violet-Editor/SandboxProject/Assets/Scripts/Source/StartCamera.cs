using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Violet;

namespace Sandbox
{
    public class StartCamera : Entity 
    {
        public float timer = 0;

        private CameraComponent cameraComponent;
        private CameraComponent ortherComponet;
        private Entity text;
        private float speed;

        void OnCreate()
        {
            cameraComponent = GetComponent<CameraComponent>();

            ortherComponet = FindEntityByName("CameraA").GetComponent<CameraComponent>();
            
            text = FindEntityByName("Text");
            speed = 30.0f / 3.0f;

        }
        void OnUpdate(float ts) 
        {

            if(timer < 2.0f)
            {
                Vector3  v3 = text.Translation;
                v3.Z -= speed * ts;
                text.Translation = v3;
                
                timer += ts;
                return;
            }
            text.Translation = new Vector3(0,0,-100);
            cameraComponent.Primary = false;
            ortherComponet.Primary = true;
            
        }

    }
}
