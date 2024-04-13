using Sandbox;
using System;
using System.Collections;
using System.Collections.Generic;
using Violet;
namespace Sandbox
{
    public class ParticleProps
    {
        public Vector2 Position;
        public Vector2 Velocity, VelocityVariation;
        public Vector4 ColorBegin, ColorEnd;
        public float SizeBegin, SizeEnd, SizeVariation;
        public float LifeTime;

        public ParticleProps() 
        {
            LifeTime = 1.0f;
        }
    };
    public class Particle
    {
        public Vector2 Position;
        public Vector2 Velocity, VelocityVariation;
        public Vector4 ColorBegin, ColorEnd;
        public float Rotation;    
        public float SizeBegin, SizeEnd, SizeVariation;

        public float LifeTime;         
        public float LifeRemaining;    

        public bool Active;            

        public Particle()
        {
            // Position = new Vector2();
            // Velocity = new Vector2();
            Rotation = 0.0f;

            LifeTime = 1.0f;
            LifeRemaining = 0.0f;

            Active = false;

           ColorBegin = new Vector4(254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f );
           ColorEnd = new Vector4(254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f);
           SizeBegin = 0.1f;
           SizeVariation = 0.3f;
           SizeEnd = 0.0f;
           LifeTime = 1.0f;
           Velocity = new Vector2(0.0f, 0.0f);
           VelocityVariation = new Vector2(3.0f, 1.0f);
           Position = new Vector2(0.0f, 0.0f);
        }
    };

    public class ParticleSystem : Entity
    {
        private List<Entity> m_EntityePool = new List<Entity>();
        private List<Particle> m_ParticlePool = new List<Particle>();
        private int m_PoolIndex = 999;
        private ParticleProps m_ParticleProps;

        void OnCreate()
        {
            for (int i = 0; i < 100; i++)
            {
                Entity entity = CreateEntity("No." + i);

                m_EntityePool.Add(entity);
                if(entity.AddComponent<SpriteRendererComponent>())
                {
                    entity.GetComponent<SpriteRendererComponent>().Texture2D = "Resources/Icons/PlayButton.png";
                    Console.WriteLine(1);
                }

                m_ParticlePool.Add(new Particle());
            }
        }

        void OnUpdate(float ts)
        {
            if (Input.IsMouseButtonDown(MouseButtonCode.ButtonLeft))
            {
                // 点击屏幕，获取鼠标位置，将其转换世界坐标（1）
                Vector2 pos = Input.GetMouseImGuiPosition();

                m_ParticleProps.Position = pos;
                for (int i = 0; i < 5; i++)
                    // 将这个世界坐标位置设置为粒子的**发射点**（2），这发射点进行**准备**发射多个对应的**粒子**（3）
                    Emit(m_ParticleProps);
            }

            OnUpdateParticle(ts);

        }

        void OnUpdateParticle(float ts)
        {
            for (int i = 0; i < m_ParticlePool.Count; i++)
            {
                var particle = m_ParticlePool[i];
                if (!particle.Active)
                    continue;

                if (particle.LifeRemaining <= 0.0f)
                {
                    particle.Active = false;
                    continue;
                }

               particle.LifeRemaining -= ts;
               particle.Position += m_ParticlePool[i].Velocity * (float)ts;
               particle.Rotation += 0.01f * ts;

                // Render

                float life = particle.LifeRemaining / particle.LifeTime;
                // start + (end - start) * t;
                Vector4 color = particle.ColorBegin + (particle.ColorEnd - particle.ColorBegin) * life;
                float sizeLerp = particle.SizeEnd + (particle.SizeBegin - particle.SizeEnd) * life;

                var tc = m_EntityePool[i].GetComponent<TransformComponent>();
                tc.Translation = new Vector3(particle.Position.X, particle.Position.Y, 0);
                tc.Scale = new Vector3(sizeLerp, sizeLerp, 1);
                tc.Rotation = new Vector3(0, 0, particle.Rotation);

            }
        }



        public void Emit(ParticleProps particleProps)
        {
            Random random = new Random();
            Particle particle = m_ParticlePool[m_PoolIndex];
            particle.Active = true;
            particle.Position = particleProps.Position;
            particle.Rotation = (float)(random.NextDouble() * 2.0f * Math.PI);

            // Velocity
            particle.Velocity = particleProps.Velocity;

            particle.Velocity.X += (float)(particleProps.VelocityVariation.X * (random.NextDouble() - 0.5f));
            particle.Velocity.Y += (float)(particleProps.VelocityVariation.Y * (random.NextDouble() - 0.5f));

            // Color
            particle.ColorBegin = particleProps.ColorBegin;
            particle.ColorEnd = particleProps.ColorEnd;

            particle.LifeTime = particleProps.LifeTime;
            particle.LifeRemaining = particleProps.LifeTime;
            particle.SizeBegin = (float)(particleProps.SizeBegin + particleProps.SizeVariation * (random.NextDouble() - 0.5f));
            particle.SizeEnd = particleProps.SizeEnd;


            m_PoolIndex = --m_PoolIndex % m_ParticlePool.Count;
            //std::cout << m_PoolIndex << std::endl;
        }
    }
}
