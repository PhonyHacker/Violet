using System;
using System.Runtime.CompilerServices;

namespace Violet
{
	public class Entity
	{
		protected Entity() { ID = 0; } 

		internal Entity(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;

		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 result);
				return result;
			}
			set
			{
				InternalCalls.TransformComponent_SetTranslation(ID, ref value);
			}
		}

		public Vector3 Rotation
		{
            get
            {
                InternalCalls.TransformComponent_GetRotation(ID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls.TransformComponent_SetRotation(ID, ref value);
            }
        }


        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(ID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls.TransformComponent_SetScale(ID, ref value);
            }
        }

        public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent(ID, componentType);
		}


        public bool AddComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            if (InternalCalls.Entity_HasComponent(ID, componentType))
                return false;

            return InternalCalls.Entity_AddComponent(ID, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;

			T component = new T() { Entity = this };
			return component;
		}

        public Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }

		public Entity CreateEntity(string name)
		{
            ulong entityID = InternalCalls.Entity_CreateEntity(name);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }

        public bool DeleteEntity(string name)
        {
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
			if (entityID == 0)
			{
				Console.WriteLine("entityID = 0");
				return false;
			}

			InternalCalls.Entity_DeleteEntity(entityID);
            return true;
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.GetScriptInstance(ID);
            return instance as T;
        }

    }

}
