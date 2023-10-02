using System.Runtime.CompilerServices;

namespace Aurora {
    public static class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern Entity CreateEntity(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern Entity GetEntity(uint id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void TransformGetPosition(uint id,  out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void TransformSetPosition(uint id, ref Vector3 position);
    }
}