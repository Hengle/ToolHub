using System.Runtime.InteropServices;
unsafe static class Memory
{
    [DllImport("VEngine_DLL.dll")]
    public static extern void vengine_init_malloc();

    [DllImport("VEngine_DLL.dll")]
    public static extern void* vengine_malloc(ulong size);
    [DllImport("VEngine_DLL.dll")]
    public static extern void vengine_free(void* ptr);
    [DllImport("VEngine_DLL.dll")]
    public static extern void* vengine_realloc(void* ptr, ulong size);
    [DllImport("VEngine_DLL.dll")]
    public static extern void vengine_memcpy(void* dest, void* src, ulong sz);
    [DllImport("VEngine_DLL.dll")]
    public static extern void vengine_memset(void* dest, byte b, ulong sz);
    [DllImport("VEngine_DLL.dll")]
    public static extern void vengine_memmove(void* dest, void* src, ulong sz);

    private static ulong RemakeSerArray(ref byte[] result, ulong newSize, int initCapacity)
    {
        if (result == null || (ulong)result.LongLength < (sizeof(ulong) + newSize))
        {
            result = new byte[sizeof(ulong) + System.Math.Max(newSize, (ulong)initCapacity)];
            fixed (byte* ptr = result)
            {
                ref var p = ref *(ulong*)ptr;
                p = newSize;
                return sizeof(ulong);
            }
        }
        else
        {
            fixed (byte* ptr = result)
            {
                ref var p = ref *(ulong*)ptr;
                var lastP = p;
                p += newSize;
                var arrLen = result.LongLength - sizeof(ulong);
                if ((ulong)arrLen < p)
                {
                    do
                    {
                        arrLen *= 2;
                    } while ((ulong)arrLen < p);
                    byte[] newResult = new byte[arrLen + sizeof(ulong)];
                    fixed (void* dest = newResult)
                    {
                        vengine_memcpy(dest, ptr, lastP + sizeof(ulong));
                    }
                    result = newResult;
                }
                return lastP + sizeof(ulong);
            }
        }
    }
    static void M_Serialize<T>(in T data, byte* dst) where T : unmanaged
    {
        fixed (void* src = &data)
        {
            vengine_memcpy(dst, src, (ulong)sizeof(T));
        }
    }
    public static ulong Serialize<T>(ref this T data, ref byte[] result, int initCapacity = 1024) where T : unmanaged
    {
        var ofst = RemakeSerArray(ref result, (ulong)sizeof(T), initCapacity);
        fixed (byte* dst = result)
        {
            M_Serialize(data, dst + ofst);
            return ofst + (ulong)sizeof(T);
        }
    }
    public static ulong Serialize(this string s, ref byte[] result, int initCapacity = 1024)
    {
        ulong cpyLen = (ulong)s.Length * sizeof(char);
        var ofst = RemakeSerArray(ref result, sizeof(uint) + cpyLen, initCapacity);
        fixed (byte* dst = result)
        fixed (char* src = s)
        {
            M_Serialize((uint)s.Length, dst + ofst);
            ofst += sizeof(uint);
            vengine_memcpy(dst + ofst, src, cpyLen);
            return ofst + cpyLen;
        }
    }
    public static ulong Serialize<T>(this T[] data, ref byte[] result, int initCapacity = 1024) where T : unmanaged
    {
        ulong cpyLen = (ulong)data.LongLength * (ulong)sizeof(T);
        var ofst = RemakeSerArray(ref result, sizeof(ulong) + cpyLen, initCapacity);
        fixed (byte* dst = result)
        fixed (void* src = data)
        {
            M_Serialize((ulong)data.Length, dst + ofst);
            ofst += sizeof(ulong);
            vengine_memcpy(dst + ofst, src, cpyLen);
            return ofst + cpyLen;
        }
    }

    public static void DeSer<T>(ref this T result, byte[] arr, ref ulong offset) where T : unmanaged
    {
        fixed (byte* ptr = arr)
        {
            DeSer<T>(ref result, ptr, ref offset);
        }
    }
    public static void DeSer<T>(ref this T result, byte* ptr, ref ulong offset) where T : unmanaged
    {
        ulong arrSize = *(ulong*)ptr;

        if (offset + (ulong)sizeof(T) > arrSize)
        {
            throw new System.IndexOutOfRangeException("Try de-serialize array without enough space");
        }
        fixed (void* dest = &result)
        {
            vengine_memcpy(dest, ptr + sizeof(ulong) + offset, (ulong)sizeof(T));
            offset += (ulong)sizeof(T);
        }

    }
    public static string DeSerString(byte[] arr, ref ulong offset)
    {
        fixed (byte* ptr = arr)
        {
            return DeSerString(ptr, ref offset);
        }
    }
    public static string DeSerString(byte* ptr, ref ulong offset)
    {
        uint size = 0;
        size.DeSer(ptr, ref offset);
        ulong cpyLen = size * sizeof(char);
        byte* newPtr = ptr + sizeof(ulong) + offset;
        ulong arrSize = *(ulong*)ptr;
        if (offset + cpyLen > arrSize)
        {
            throw new System.IndexOutOfRangeException("Try de-serialize array without enough space");
        }
        string newStr = new string((char*)newPtr, 0, (int)size);
        offset += (ulong)size * sizeof(char);
        return newStr;
    }
    public static T[] DeSerArray<T>(byte* ptr, ref ulong offset) where T : unmanaged
    {
        ulong size = 0;
        size.DeSer(ptr, ref offset);
        ulong cpyLen = size * (ulong)sizeof(T);

        byte* newPtr = ptr + sizeof(ulong) + offset;
        ulong arrSize = *(ulong*)ptr;
        if (offset + cpyLen > arrSize)
        {
            throw new System.IndexOutOfRangeException("Try de-serialize array without enough space");
        }
        T[] newArr = new T[size];
        fixed (void* dest = newArr)
        {
            vengine_memcpy(dest, newPtr, cpyLen);
        }
        offset += cpyLen;
        return newArr;
    }
    public static T[] DeSerArray<T>(byte[] arr, ref ulong offset) where T : unmanaged
    {
        fixed (byte* ptr = arr)
        {
            return DeSerArray<T>(ptr, ref offset);
        }
    }
}