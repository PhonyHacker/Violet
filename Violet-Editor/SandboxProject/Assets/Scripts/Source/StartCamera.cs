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
        private TextComponent textComponent;
        private float speed;

        void OnCreate()
        {
            cameraComponent = GetComponent<CameraComponent>();

            ortherComponet = FindEntityByName("CameraA").GetComponent<CameraComponent>();
            
            text = FindEntityByName("Text");
            textComponent = text.GetComponent<TextComponent>();
            speed = 30.0f / 3.0f;

        }
        void OnUpdate(float ts) 
        {

            if(timer < 2.0f)
            {
                if(timer > 1.0f)
                {
                    textComponent.Color = new Vector4(0.8f, 0.0f, 0.0f, 1.0f);
                    textComponent.Text = "Enjoy Yourself !";
                }

                Vector3  v3 = text.Translation;
                v3.Z -= speed * ts;
                text.Translation = v3;
                
                timer += ts;
                return;
            }

            if(text!=null)
            {
                bool res = DeleteEntity("Text");
                // Console.WriteLine(res);
                
                text = null;
            }

            cameraComponent.Primary = false;
            ortherComponet.Primary = true;
            
        }

    }
}
