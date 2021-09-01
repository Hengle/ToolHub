using System.Runtime.InteropServices;
using System;
using System.Threading;
using System.Collections;
using System.Collections.Generic;
namespace vstd
{
    [Network.VSerializable]
    public unsafe struct Guid
    {
        public ulong data0;
        public ulong data1;
        public Guid(ulong data0, ulong data1)
        {
            this.data0 = data0;
            this.data1 = data1;
        }
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
                vguid_to_string(ptr, bytes, false);
            }
            return new string(bytes, 0, 32);
        }
        [DllImport("VEngine_DLL.dll")]
        static extern void vguid_get_new(
        Guid* guidData);
        [DllImport("VEngine_DLL.dll")]
        public static extern void vguid_get_from_string(
        sbyte* str,
        int strLen,
        Guid* guidData);
        [DllImport("VEngine_DLL.dll")]
        public static extern void vguid_to_string(
        Guid* guidData,
        sbyte* result,
        bool upper);

        [DllImport("VEngine_DLL.dll")]
        static extern void parse_unity_metafile(
        byte* fileData,
        ulong fileSize,
        Guid* guid);

        [DllImport("Yaml_CPP.dll")]
        static extern uint read_unity_file(
        IntPtr callBack,
        char* filePath,
        uint filePathLen);

        [DllImport("Yaml_CPP.dll")]
        static extern void vguid_to_compress_str(
        Guid* guidData,
        byte* result);
        static ThreadLocal<List<string>> localGuidList = new ThreadLocal<List<string>>();
        delegate void GetGuidCallBackType(IntPtr ptr, uint sz);
        static GetGuidCallBackType getGuidCallback = (charPtr, charSize) =>
        {
            var value = localGuidList.Value;
            sbyte* bytes = (sbyte*)charPtr.ToPointer();
            value.Add(new string(bytes, 0, (int)charSize));
        };
        public string ToCompressString()
        {
            byte* bytes = stackalloc byte[20];
            fixed (Guid* ptr = &this)
            {
                vguid_to_compress_str(
                    ptr,
                    bytes);
            }
            return new string((sbyte*)bytes, 0, 20);
        }
        public static uint TryReadUnityAssetRefGuid(
            string filePath,
            List<string> result)
        {
            result.Clear();
            localGuidList.Value = result;
            fixed (char* ptr = filePath)
            {
                uint i;
                if ((i = read_unity_file(Marshal.GetFunctionPointerForDelegate(getGuidCallback), ptr, (uint)filePath.Length)) != 0)
                    return i;
            }
            localGuidList.Value = null;
            return 0;
        }

        public static Guid GetGuidFromUnityMeta(byte* fileData, ulong fileSize)
        {
            Guid g;
            parse_unity_metafile(fileData, fileSize, &g);
            return g;
        }
        public static Guid GetGuidFromUnityMeta(string filePath)
        {
            var bytes = System.IO.File.ReadAllBytes(filePath);
            fixed (byte* bs = bytes)
            {
                return GetGuidFromUnityMeta(bs, (ulong)bytes.LongLength);
            }
        }
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