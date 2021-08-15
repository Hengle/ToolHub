using System.Runtime.InteropServices;
namespace vstd
{
    public unsafe struct MD5
    {
        ulong data0;
        ulong data1;
        [DllImport("VEngine_DLL.dll")]
        static extern void unity_get_md5(
        byte* data,
        ulong dataLength,
        byte* destData);
        public MD5(byte* data, ulong dataLength)
        {
            data0 = 0;
            data1 = 0;
            fixed (MD5* b = &this)
            {
                unity_get_md5(data, dataLength, (byte*)b);
            }
        }
        public MD5(string s)
        {
            data0 = 0;
            data1 = 0;
            fixed (MD5* b = &this)
            fixed (char* c = s)
            {
                unity_get_md5((byte*)c, (ulong)s.Length * sizeof(char), (byte*)b);
            }
        }
        public MD5(byte[] arr)
        {
            data0 = 0;
            data1 = 0;
            fixed (byte* ab = arr)
            fixed (MD5* b = &this)
            {
                unity_get_md5(ab, (ulong)arr.Length, (byte*)b);
            }
        }
        public override int GetHashCode()
        {
            return data0.GetHashCode() ^ (data1.GetHashCode() << 4);
        }
        public override bool Equals(object obj)
        {
            MD5 d = (MD5)obj;
            return d.data0 == data0 && d.data1 == data1;
        }
        public static bool operator ==(in MD5 g0, in MD5 g1)
        {
            return g0.data0 == g1.data0 && g0.data1 == g1.data1;
        }
        public static bool operator !=(in MD5 g0, in MD5 g1)
        {
            return !(g0 == g1);

        }
        public override string ToString()
        {
            sbyte* bytes = stackalloc sbyte[32];
            fixed (MD5* ptr = &this)
            {
                Guid.vguid_to_string((Guid*)ptr, bytes, true);
            }
            return new string(bytes, 0, 32);
        }
    }
}