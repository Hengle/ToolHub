using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Threading;
namespace vstd
{
    public struct FatPtr
    {
        public IntPtr ptr;
        public bool isManaging;
    }
    public unsafe struct SerializeObject : IDisposable
    {
        IntPtr ptr;
        public SerializeObject(bool initialize)
        {
            if (initialize)
            {
                fixed (IntPtr* pp = &ptr)
                    db_get_new(pp);
            }
            else
            {
                ptr = new IntPtr(null);
            }
        }
        public void Dispose()
        {
            if (ptr.ToPointer() != null)
                db_dispose(ptr);
        }
        public void Init()
        {
            if (ptr.ToPointer() == null)
            {
                fixed (IntPtr* pp = &ptr)
                    db_get_new(pp);
            }
        }
        public delegate void SerializeCallback(byte* bPtr, ulong byteSize);
        public void Serialize(
            SerializeCallback callback)
        {
            db_serialize(ptr, Marshal.GetFunctionPointerForDelegate(callback));
        }
        static ThreadLocal<string> tLocalFilePath = new ThreadLocal<string>();
        static SerializeCallback writeToFile = (bPtr, byteSize) =>
        {
            using (System.IO.BinaryWriter bin = new System.IO.BinaryWriter(new System.IO.FileStream(tLocalFilePath.Value, System.IO.FileMode.Create)))
            {
                bin.Write(new ReadOnlySpan<byte>(bPtr, (int)byteSize));
            }
        };
        public void SerializeToFile(string filePath)
        {
            tLocalFilePath.Value = filePath;
            Serialize(writeToFile);
        }
        //public void DeSerialize()
        public SerializeDict GetRootNode()
        {
            FatPtr ptr = new FatPtr();
            db_get_rootnode(this.ptr, &ptr.ptr);
            return new SerializeDict(ptr);
        }
        public SerializeDict CreateNewDict()
        {
            FatPtr ptr = new FatPtr { isManaging = true };
            db_create_dict(this.ptr, &ptr.ptr);
            return new SerializeDict(ptr);
        }
        public SerializeArray CreateNewArray()
        {
            FatPtr ptr = new FatPtr { isManaging = true };
            db_create_array(this.ptr, &ptr.ptr);
            return new SerializeArray(ptr);
        }
        public void Read(Span<byte> sp)
        {
            fixed(byte* b = sp)
            {
                db_deser(ptr, b, (ulong)sp.Length);
            }
        }
        public void ReadFromFile(string filePath)
        {
            byte[] b = System.IO.File.ReadAllBytes(filePath);
            Read(b);
        }
        [DllImport("VEngine_Database.dll")]
        static extern void db_get_new(IntPtr* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dispose(IntPtr ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_get_rootnode(IntPtr db, IntPtr* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_create_dict(IntPtr db, IntPtr* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_create_array(IntPtr db, IntPtr* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_serialize(IntPtr db, IntPtr callback);
        [DllImport("VEngine_Database.dll")]
        static extern void db_deser(IntPtr db, byte* ptr, ulong sz);
    }
    unsafe struct CSString
    {
        public byte* bytes;
        public ulong len;
    }
    enum KeyType : uint
    {
        Int64,
        String,
        Guid
    };
    enum ValueType : uint
    {
        Int64,
        Double,
        String,
        Dict,
        Array,
        Guid
    };
    public unsafe struct SerializeDict : IDisposable
    {

        FatPtr fatPtr;
        [DllImport("VEngine_Database.dll")]
        static extern void db_dispose_dict(IntPtr ptr);

        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_set(IntPtr dict, void* keyPtr, KeyType keyType, void* valuePtr, ValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_get(IntPtr dict, void* keyPtr, KeyType keyType, ValueType targetValueType, void* valuePtr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_remove(IntPtr dict, void* keyPtr, KeyType keyType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_len(IntPtr dict, int* sz);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_itebegin(IntPtr dict, Iterator* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_iteend(IntPtr dict, Iterator* end, bool* result);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_next(Iterator* end);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_get(Iterator ite, void* valuePtr, ValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_getkey(Iterator ite, void* keyPtr, KeyType valueType);
        public struct Iterator
        {
            ulong placeHolder;
            public static bool operator ==(Iterator a, Iterator b)
            {
                return a.placeHolder == b.placeHolder;
            }
            public static bool operator !=(Iterator a, Iterator b)
            {
                return a.placeHolder != b.placeHolder;
            }
        }
        public Iterator Begin()
        {
            Iterator i;
            db_dict_itebegin(fatPtr.ptr, &i);
            return i;
        }
        public bool End(Iterator ite)
        {
            bool v;
            db_dict_iteend(fatPtr.ptr, &ite, &v);
            return v;

        }
        public void GetNext(ref Iterator ite)
        {
            fixed (Iterator* ptr = &ite)
            {
                db_dict_ite_next(ptr);
            }
        }
        public void Dispose()
        {
            if (fatPtr.isManaging)
            {
                db_dispose_dict(fatPtr.ptr);
                fatPtr.isManaging = false;
                fatPtr.ptr = new IntPtr(null);
            }
        }
        public SerializeDict(FatPtr ptr)
        {
            fatPtr = ptr;
        }
        public struct MovedHandle
        {
            public IntPtr holder;
        }
        public int Length
        {
            get
            {
                int v;
                db_dict_len(fatPtr.ptr, &v);
                return v;
            }
        }
        public MovedHandle Move()
        {
            if (!fatPtr.isManaging)
            {
                throw new AccessViolationException("try move an object without life-right!");
            }
            MovedHandle handle = new MovedHandle
            {
                holder = fatPtr.ptr
            };
            fatPtr.isManaging = false;
            return handle;
        }
        public long GetInt(Iterator ite)
        {
            long v;
            db_dict_ite_get(ite, &v, ValueType.Int64);
            return v;
        }
        public long GetKeyInt(Iterator ite)
        {
            long v;
            db_dict_ite_getkey(ite, &v, KeyType.Int64);
            return v;
        }
        public long GetInt(long intKey)
        {
            long v;
            db_dict_get(fatPtr.ptr, &intKey, KeyType.Int64, ValueType.Int64, &v);
            return v;
        }
        public long GetInt(string strKey)
        {
            long v;
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_get(fatPtr.ptr, &str, KeyType.String, ValueType.Int64, &v);
            return v;
        }
        public long GetInt(Guid guidKey)
        {
            long v;
            db_dict_get(fatPtr.ptr, &guidKey, KeyType.Guid, ValueType.Int64, &v);
            return v;
        }
        public double GetFloat(Iterator ite)
        {
            double v;
            db_dict_ite_get(ite, &v, ValueType.Double);
            return v;
        }
        public double GetFloat(long intKey)
        {
            double v;
            db_dict_get(fatPtr.ptr, &intKey, KeyType.Int64, ValueType.Double, &v);
            return v;
        }
        public double GetFloat(string strKey)
        {
            double v;
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_get(fatPtr.ptr, &str, KeyType.String, ValueType.Double, &v);
            return v;
        }
        public double GetFloat(Guid guidKey)
        {
            double v;
            db_dict_get(fatPtr.ptr, &guidKey, KeyType.Guid, ValueType.Double, &v);
            return v;
        }
        public string GetString(Iterator ite)
        {
            CSString v;
            db_dict_ite_get(ite, &v, ValueType.String);
            return new string((sbyte*)v.bytes, 0, (int)v.len);
        }
        public string GetKeyString(Iterator ite)
        {
            CSString v;
            db_dict_ite_getkey(ite, &v, KeyType.String);
            return new string((sbyte*)v.bytes, 0, (int)v.len);
        }
        public string GetString(long intKey)
        {
            CSString v;
            db_dict_get(fatPtr.ptr, &intKey, KeyType.Int64, ValueType.String, &v);
            return new string((sbyte*)v.bytes, 0, (int)v.len);
        }
        public string GetString(string strKey)
        {
            CSString v;
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_get(fatPtr.ptr, &str, KeyType.String, ValueType.String, &v);
            return new string((sbyte*)v.bytes, 0, (int)v.len);

        }
        public string GetString(Guid guidKey)
        {
            CSString v;
            db_dict_get(fatPtr.ptr, &guidKey, KeyType.Guid, ValueType.String, &v);
            return new string((sbyte*)v.bytes, 0, (int)v.len);

        }
        public Guid GetGuid(Iterator ite)
        {
            Guid v;
            db_dict_ite_get(ite, &v, ValueType.Guid);
            return v;
        }
        public Guid GetKeyGuid(Iterator ite)
        {
            Guid v;
            db_dict_ite_getkey(ite, &v, KeyType.Guid);
            return v;
        }
        public Guid GetGuid(long intKey)
        {
            Guid v;
            db_dict_get(fatPtr.ptr, &intKey, KeyType.Int64, ValueType.Guid, &v);
            return v;
        }
        public Guid GetGuid(string strKey)
        {
            Guid v;
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_get(fatPtr.ptr, &str, KeyType.String, ValueType.Guid, &v);
            return v;
        }
        public Guid GetGuid(Guid guidKey)
        {
            Guid v;
            db_dict_get(fatPtr.ptr, &guidKey, KeyType.Guid, ValueType.Guid, &v);
            return v;
        }
        public SerializeArray GetArray(Iterator ite)
        {
            IntPtr v;
            db_dict_ite_get(ite, &v, ValueType.Array);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }

        public SerializeArray GetArray(long intKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &intKey, KeyType.Int64, ValueType.Array, &v);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeArray GetArray(string strKey)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            IntPtr v;
            db_dict_get(fatPtr.ptr, &str, KeyType.String, ValueType.Array, &v);
            if (v.ToPointer() == null)
            {
                var x = v;
            }
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeArray GetArray(Guid guidKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &guidKey, KeyType.Guid, ValueType.Array, &v);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }

        public SerializeDict GetDict(Iterator ite)
        {
            IntPtr v;
            db_dict_ite_get(ite, &v, ValueType.Dict);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeDict GetDict(long intKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &intKey, KeyType.Int64, ValueType.Dict, &v);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeDict GetDict(string strKey)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            IntPtr v;
            db_dict_get(fatPtr.ptr, &str, KeyType.String, ValueType.Dict, &v);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeDict GetDict(Guid guidKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &guidKey, KeyType.Guid, ValueType.Dict, &v);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }

        public void SetInt(long intKey, long value)
        {
            db_dict_set(fatPtr.ptr, &intKey, KeyType.Int64, &value, ValueType.Int64);
        }
        public void SetInt(string strKey, long value)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_set(fatPtr.ptr, &str, KeyType.String, &value, ValueType.Int64);

        }
        public void SetInt(Guid guidKey, long value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, KeyType.Guid, &value, ValueType.Int64);

        }
        public void SetFloat(long intKey, double value)
        {
            db_dict_set(fatPtr.ptr, &intKey, KeyType.Int64, &value, ValueType.Double);
        }
        public void SetFloat(string strKey, double value)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_set(fatPtr.ptr, &str, KeyType.String, &value, ValueType.Double);

        }
        public void SetFloat(Guid guidKey, double value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, KeyType.Guid, &value, ValueType.Double);
        }

        public void SetString(long intKey, string value)
        {
            byte* bytes = stackalloc byte[value.Length];
            for (int i = 0; i < value.Length; ++i)
            {
                bytes[i] = (byte)value[i];
            }
            CSString strValue = new CSString
            {
                bytes = bytes,
                len = (ulong)value.Length
            };
            db_dict_set(fatPtr.ptr, &intKey, KeyType.Int64, &strValue, ValueType.String);
        }
        public void SetString(string strKey, string value)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };

            byte* bytesValue = stackalloc byte[value.Length];
            for (int i = 0; i < value.Length; ++i)
            {
                bytesValue[i] = (byte)value[i];
            }
            CSString strValue = new CSString
            {
                bytes = bytesValue,
                len = (ulong)value.Length
            };
            db_dict_set(fatPtr.ptr, &str, KeyType.String, &strValue, ValueType.String);
        }
        public void SetString(Guid guidKey, string value)
        {
            byte* bytesValue = stackalloc byte[value.Length];
            for (int i = 0; i < value.Length; ++i)
            {
                bytesValue[i] = (byte)value[i];
            }
            CSString strValue = new CSString
            {
                bytes = bytesValue,
                len = (ulong)value.Length
            };
            db_dict_set(fatPtr.ptr, &guidKey, KeyType.Guid, &strValue, ValueType.String);
        }

        public void SetGuid(long intKey, Guid value)
        {
            db_dict_set(fatPtr.ptr, &intKey, KeyType.Int64, &value, ValueType.Guid);
        }
        public void SetGuid(string strKey, Guid value)
        {

            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_set(fatPtr.ptr, &str, KeyType.String, &value, ValueType.Guid);

        }
        public void SetGuid(Guid guidKey, Guid value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, KeyType.Guid, &value, ValueType.Guid);
        }

        public void SetArray(long intKey, SerializeArray.MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &intKey, KeyType.Int64, &value, ValueType.Array);
        }
        public void SetArray(string strKey, SerializeArray.MovedHandle value)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_set(fatPtr.ptr, &str, KeyType.String, &value, ValueType.Array);
        }
        public void SetArray(Guid guidKey, SerializeArray.MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, KeyType.Guid, &value, ValueType.Array);
        }
        public void SetDict(long intKey, MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &intKey, KeyType.Int64, &value, ValueType.Dict);
        }
        public void SetDict(string strKey, MovedHandle value)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_set(fatPtr.ptr, &str, KeyType.String, &value, ValueType.Dict);
        }
        public void SetDict(Guid guidKey, MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, KeyType.Guid, &value, ValueType.Dict);
        }

        public void Remove(long intKey)
        {
            db_dict_remove(fatPtr.ptr, &intKey, KeyType.Int64);
        }
        public void Remove(string strKey)
        {
            byte* bytes = stackalloc byte[strKey.Length];
            for (int i = 0; i < strKey.Length; ++i)
            {
                bytes[i] = (byte)strKey[i];
            }
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)strKey.Length
            };
            db_dict_remove(fatPtr.ptr, &str, KeyType.String);
        }
        public void Remove(Guid guidKey)
        {
            db_dict_remove(fatPtr.ptr, &guidKey, KeyType.Guid);
        }

    }
    public unsafe struct SerializeArray : IDisposable
    {
        public FatPtr fatPtr;
        public SerializeArray(FatPtr ptr)
        {
            fatPtr = ptr;
        }
        public struct MovedHandle
        {
            public IntPtr holder;
        }
        public MovedHandle Move()
        {
            if (!fatPtr.isManaging)
            {
                throw new AccessViolationException("try move an object without life-right!");
            }
            MovedHandle handle = new MovedHandle
            {
                holder = fatPtr.ptr
            };
            fatPtr.isManaging = false;
            return handle;
        }
        public void Dispose()
        {
            if (fatPtr.isManaging)
            {
                db_dispose_arr(fatPtr.ptr);
                fatPtr.isManaging = false;
                fatPtr.ptr = new IntPtr(null);
            }
        }
        public struct Iterator
        {
            ulong placeHolder;
            public static bool operator ==(Iterator a, Iterator b)
            {
                return a.placeHolder == b.placeHolder;
            }
            public static bool operator !=(Iterator a, Iterator b)
            {
                return a.placeHolder != b.placeHolder;
            }
        }
        [DllImport("VEngine_Database.dll")]
        static extern void db_dispose_arr(IntPtr arr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_len(IntPtr arr, int* sz);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_get_value(IntPtr arr, int index, void* valuePtr, ValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_set_value(IntPtr arr, int index, void* valuePtr, ValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_add_value(IntPtr arr, void* valuePtr, ValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_remove(IntPtr arr, int index);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_itebegin(IntPtr dict, Iterator* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_iteend(IntPtr dict, Iterator* end, bool* result);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_ite_next(IntPtr dict, Iterator* end);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_ite_get(Iterator ite, void* valuePtr, ValueType valueType);
        public int Length
        {
            get
            {
                int v;
                db_arr_len(fatPtr.ptr, &v);
                return v;
            }
        }
        public Iterator Begin()
        {
            Iterator i;
            db_arr_itebegin(fatPtr.ptr, &i);
            return i;
        }
        public bool End(Iterator ite)
        {
            bool v;
            db_arr_iteend(fatPtr.ptr, &ite, &v);
            return v;
        }
        public void GetNext(ref Iterator ite)
        {
            fixed (Iterator* ptr = &ite)
            {
                db_arr_ite_next(fatPtr.ptr, ptr);
            }
        }
        public long GetInt(int index)
        {
            long v;
            db_arr_get_value(fatPtr.ptr, index, &v, ValueType.Int64);
            return v;
        }
        public long GetInt(Iterator ite)
        {
            long v;
            db_arr_ite_get(ite, &v, ValueType.Int64);
            return v;
        }

        public string GetString(int index)
        {
            CSString v;
            db_arr_get_value(fatPtr.ptr, index, &v, ValueType.String);
            return new string((sbyte*)v.bytes, 0, (int)v.len);
        }
        public string GetString(Iterator ite)
        {
            CSString v;
            db_arr_ite_get(ite, &v, ValueType.String);
            return new string((sbyte*)v.bytes, 0, (int)v.len);
        }

        public double GetFloat(int index)
        {
            double v;
            db_arr_get_value(fatPtr.ptr, index, &v, ValueType.Double);
            return v;
        }
        public double GetFloat(Iterator ite)
        {
            double v;
            db_arr_ite_get(ite, &v, ValueType.Double);
            return v;
        }
        public SerializeDict GetDict(int index)
        {
            IntPtr v;
            db_arr_get_value(fatPtr.ptr, index, &v, ValueType.Dict);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeDict GetDict(Iterator ite)
        {
            IntPtr v;
            db_arr_ite_get(ite, &v, ValueType.Dict);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeArray GetArray(int index)
        {
            IntPtr v;
            db_arr_get_value(fatPtr.ptr, index, &v, ValueType.Array);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeArray GetArray(Iterator ite)
        {
            IntPtr v;
            db_arr_ite_get(ite, &v, ValueType.Array);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }
        public Guid GetGuid(int index)
        {
            Guid v;
            db_arr_get_value(fatPtr.ptr, index, &v, ValueType.Guid);
            return v;
        }
        public Guid GetGuid(Iterator ite)
        {
            Guid v;
            db_arr_ite_get(ite, &v, ValueType.Guid);
            return v;
        }

        public void AddInt(long value)
        {
            db_arr_add_value(fatPtr.ptr, &value, ValueType.Int64);
        }
        public void AddFloat(double value)
        {
            db_arr_add_value(fatPtr.ptr, &value, ValueType.Double);
        }
        public void AddString(string value)
        {
            byte* bytesValue = stackalloc byte[value.Length];
            for (int i = 0; i < value.Length; ++i)
            {
                bytesValue[i] = (byte)value[i];
            }
            CSString strValue = new CSString
            {
                bytes = bytesValue,
                len = (ulong)value.Length
            };
            db_arr_add_value(fatPtr.ptr, &strValue, ValueType.String);
        }
        public void AddDict(SerializeDict.MovedHandle value)
        {
            db_arr_add_value(fatPtr.ptr, &value, ValueType.Dict);
        }
        public void AddArray(MovedHandle value)
        {
            db_arr_add_value(fatPtr.ptr, &value, ValueType.Array);
        }
        public void AddGuid(Guid value)
        {
            db_arr_add_value(fatPtr.ptr, &value, ValueType.Guid);
        }

        public void SetInt(int index, long value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, ValueType.Int64);
        }
        public void SetFloat(int index, double value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, ValueType.Double);
        }
        public void SetString(int index, string value)
        {
            byte* bytesValue = stackalloc byte[value.Length];
            for (int i = 0; i < value.Length; ++i)
            {
                bytesValue[i] = (byte)value[i];
            }
            CSString strValue = new CSString
            {
                bytes = bytesValue,
                len = (ulong)value.Length
            };
            db_arr_set_value(fatPtr.ptr, index, &strValue, ValueType.String);
        }
        public void SetDict(int index, SerializeDict.MovedHandle value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, ValueType.Dict);
        }
        public void SetArray(int index, MovedHandle value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, ValueType.Array);
        }
        public void SetGuid(int index, Guid value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, ValueType.Guid);
        }
        public void Remove(int index)
        {
            db_arr_remove(fatPtr.ptr, index);
        }
    }

}