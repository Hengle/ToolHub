using System.Runtime.InteropServices;
namespace vstd
{
   public unsafe struct Guid
    {
        ulong data0;
        ulong data1;
        public override int GetHashCode()
        {
            return data0.GetHashCode() ^ (data1.GetHashCode() << 4);
        }
        public override bool Equals(object obj)
        {
            Guid d = (Guid)obj;
            return d.data0 == data0 && d.data1 == data1;
        }
        public static bool operator ==(in Guid g0, in Guid g1)
        {
            return g0.data0 == g1.data0 && g0.data1 == g1.data1;
        }
        public static bool operator !=(in Guid g0, in Guid g1)
        {
            return !(g0 == g1);

        }
        public override string ToString()
        {
            sbyte* bytes = stackalloc sbyte[32];
       
            fixed (Guid* ptr = &this)
            {
                vguid_to_string(ptr, bytes, true);
            }
            return new string(bytes, 0, 32);
        }
        [DllImport("VEngine_DLL.dll")]
        static extern void vguid_get_new(
        Guid* guidData);
        [DllImport("VEngine_DLL.dll")]
        static extern void vguid_get_from_string(
        sbyte* str,
        int strLen,
        Guid* guidData);
        [DllImport("VEngine_DLL.dll")]
        public static extern void vguid_to_string(
        Guid* guidData,
        sbyte* result,
        bool upper);
        public Guid(bool generate)
        {
            data0 = 0;
            data1 = 0;
            if (generate)
            {
                fixed (Guid* ptr = &this)
                {
                    vguid_get_new(ptr);
                };
            }
        }
        public Guid(string guidStr)
        {
            data0 = 0;
            data1 = 0;
            if (guidStr.Length != 32)
            {
                throw new System.ArgumentException("wrong format guid!");
            }
            sbyte* bytes = stackalloc sbyte[32];
            for (int i = 0; i < 32; ++i)
            {
                bytes[i] = (sbyte)guidStr[i];
            }
            fixed (Guid* ptr = &this)
            {
                vguid_get_from_string(bytes, 32, ptr);
            }
        }

    }
}