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
        public bool IsCreated
        {
            get { return ptr.ToPointer() != null; }
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
        private static ThreadLocal<byte[]> tLocalBytes = new ThreadLocal<byte[]>();
        private static SerializeCallback getBytes = (byte* bPtr, ulong byteSize) =>
        {
            byte[] bb = new byte[byteSize];
            fixed (byte* bbPtr = bb)
            {
                Native.Memory.vengine_memcpy(bbPtr, bPtr, byteSize);
            }
            tLocalBytes.Value = bb;
        };
        public byte[] SerializeToByteArray()
        {
            Serialize(getBytes);
            var v = tLocalBytes.Value;
            tLocalBytes.Value = null;
            return v;
        }
     
        public void SerializeToFile(string filePath)
        {
            byte* bytes = stackalloc byte[filePath.Length + 1];
            for (int i = 0; i < filePath.Length; ++i)
            {
                bytes[i] = (byte)filePath[i];
            }
            bytes[filePath.Length] = 0;
            CSString str = new CSString
            {
                bytes = bytes,
                len = (ulong)filePath.Length
            };
            db_serialize_tofile(ptr, str);
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
            fixed (byte* b = sp)
            {
                db_deser(ptr, b, (ulong)sp.Length);
            }
        }
        delegate void PrintDelegate(CSString str);
        static PrintDelegate printStr = (str) =>
        {
            string s = new string((sbyte*)str.bytes, 0, (int)str.len);
            Console.WriteLine(s);
        };
        public void Print()
        {
            db_print(ptr, Marshal.GetFunctionPointerForDelegate(printStr));
        }
        public void ReadFromFile(string filePath)
        {
            try
            {
                byte[] b = System.IO.File.ReadAllBytes(filePath);
                if (b.Length > 0)
                {
                    Read(b);
                }
            }
            catch
            {

            }
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
        static extern void db_serialize_tofile(IntPtr db, CSString str);

        [DllImport("VEngine_Database.dll")]
        static extern void db_deser(IntPtr db, byte* ptr, ulong sz);
        [DllImport("VEngine_Database.dll")]
        static extern void db_print(IntPtr db, IntPtr callback);
        public static BsonPlaceHolder Internal_GetKeyHolder(in BsonKey key)
        {
            BsonPlaceHolder holder;
            switch (key.type)
            {
                case BsonKeyType.Guid:
                    *(Guid*)(&holder) = (Guid)key.value;
                    break;
                case BsonKeyType.Int64:
                    *(long*)(&holder) = (long)key.value;
                    break;
                case BsonKeyType.String:
                    *(CSString*)(&holder) = (CSString)key.value;
                    break;
                default:
                    holder = new BsonPlaceHolder();
                    break;
            }
            return holder;
        }

        public static BsonKey Internal_GetKey(in BsonKeyType type, ref BsonPlaceHolder holder)
        {
            BsonKey result = new BsonKey
            {
                type = type,
                value = null
            };
            fixed (void* ptr = &holder)
            {
                switch (type)
                {
                    case BsonKeyType.Guid:
                        result.value = *(Guid*)ptr;
                        break;
                    case BsonKeyType.Int64:
                        result.value = *(long*)ptr;
                        break;
                    case BsonKeyType.String:
                        result.value = *(CSString*)ptr;
                        break;
                }
            }
            return result;
        }
        public static BsonValue Internal_GetValue(in BsonValueType type, ref BsonPlaceHolder holder)
        {
            BsonValue result = new BsonValue
            {
                type = type,
                value = null
            };
            fixed (void* ptr = &holder)
            {
                switch (type)
                {
                    case BsonValueType.Guid:
                        result.value = *(Guid*)ptr;
                        break;
                    case BsonValueType.Int64:
                        result.value = *(long*)ptr;
                        break;
                    case BsonValueType.Double:
                        result.value = *(double*)ptr;
                        break;
                    case BsonValueType.String:
                        result.value = *(CSString*)ptr;
                        break;
                    case BsonValueType.Array:
                        result.value = new SerializeArray(new FatPtr { ptr = *(IntPtr*)ptr, isManaging = false });
                        break;
                    case BsonValueType.Dict:
                        result.value = new SerializeDict(new FatPtr { ptr = *(IntPtr*)ptr, isManaging = false });
                        break;
                }
            }
            return result;

            /* BsonPlaceHolder holder;
             switch (key.type)
             {
                 case BsonValueType.Guid:
                     *(Guid*)(&holder) = (Guid)key.value;
                     break;
                 case BsonValueType.Int64:
                     *(long*)(&holder) = (long)key.value;
                     break;
                 case BsonValueType.Double:
                     *(double*)(&holder) = (double)key.value;
                     break;
                 case BsonValueType.String:
                     *(CSString*)(&holder) = (CSString)key.value;
                     break;
                 case BsonValueType.Array:
                     *(IntPtr*)(&holder) = ((SerializeArray)key.value).fatPtr.ptr;
                     break;
                 case BsonValueType.Dict:
                     *(IntPtr*)(&holder) = ((SerializeDict)key.value).fatPtr.ptr;
                     break;
                 default:
                     holder = new BsonPlaceHolder();
                     break;
             }
             return holder;*/
        }
    }
    public unsafe struct CSString
    {
        public byte* bytes;
        public ulong len;
    }
    public enum BsonKeyType : uint
    {
        Int64,
        String,
        Guid,
        None
    };

    public struct BsonKey
    {
        /**
         * long
         * CSString
         * vstd.Guid
         */
        public BsonKeyType type;
        public object value;
        public BsonKey(long v)
        {
            value = v;
            type = BsonKeyType.Int64;
        }
        public BsonKey(CSString s)
        {
            value = s;
            type = BsonKeyType.String;
        }
        public BsonKey(in Guid s)
        {
            value = s;
            type = BsonKeyType.Guid;
        }
    }

    public enum BsonValueType : uint
    {
        Int64,
        Double,
        String,
        Dict,
        Array,
        Guid,
        None
    };
    public struct BsonValue
    {
        /**
         * long
         * double
         * CSString
         * SerializeDict
         * SerializeArray
         * vstd.Guid
         */
        public BsonValueType type;
        public object value;
    }
    public struct BsonPlaceHolder
    {
        ulong v0;
        ulong v1;
        ulong v2;
    }
    public unsafe struct SerializeDict : IDisposable
    {

        public FatPtr fatPtr;
        public bool IsCreated
        {
            get { return fatPtr.ptr.ToPointer() != null; }
        }
        [DllImport("VEngine_Database.dll")]
        static extern void db_dispose_dict(IntPtr ptr);

        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_set(IntPtr dict, void* keyPtr, BsonKeyType keyType, void* valuePtr, BsonValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_get(IntPtr dict, void* keyPtr, BsonKeyType keyType, BsonValueType targetValueType, void* valuePtr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_get_variant(IntPtr dict, void* keyPtr, BsonKeyType keyType, BsonValueType* targetValueType, void* valuePtr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_remove(IntPtr dict, void* keyPtr, BsonKeyType keyType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_len(IntPtr dict, int* sz);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_itebegin(IntPtr dict, Iterator* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_iteend(IntPtr dict, Iterator* end, bool* result);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_next(Iterator* end);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_get(Iterator ite, void* valuePtr, BsonValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_get_variant(Iterator ite, void* valuePtr, BsonValueType* valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_getkey(Iterator ite, void* keyPtr, BsonKeyType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_dict_ite_getkey_variant(Iterator ite, void* keyPtr, BsonKeyType* valueType);

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
            db_dict_ite_get(ite, &v, BsonValueType.Int64);
            return v;
        }
        public BsonValue Get(Iterator ite)
        {
            BsonPlaceHolder holder;
            BsonValueType valueType;
            db_dict_ite_get_variant(ite, &holder, &valueType);
            return SerializeObject.Internal_GetValue(valueType, ref holder);
        }
        public long GetKeyInt(Iterator ite)
        {
            long v;
            db_dict_ite_getkey(ite, &v, BsonKeyType.Int64);
            return v;
        }
        public BsonKey GetKey(Iterator ite)
        {
            BsonPlaceHolder holder;
            BsonKeyType type;
            db_dict_ite_getkey_variant(ite, &holder, &type);
            return SerializeObject.Internal_GetKey(type, ref holder);
        }
        //public BsonKey GetKey
        public long GetInt(long intKey)
        {
            long v;
            db_dict_get(fatPtr.ptr, &intKey, BsonKeyType.Int64, BsonValueType.Int64, &v);
            return v;
        }
        public BsonValue Get(
            in BsonKey value)
        {
            BsonPlaceHolder keyHolder = SerializeObject.Internal_GetKeyHolder(value);
            BsonPlaceHolder valueHolder;
            BsonValueType valueType;
            db_dict_get_variant(fatPtr.ptr, &keyHolder, value.type, &valueType, &valueHolder);
            return SerializeObject.Internal_GetValue(valueType, ref valueHolder);

        }

        public BsonValue Get(
           in string strKey)
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
            BsonPlaceHolder keyHolder = SerializeObject.Internal_GetKeyHolder(new BsonKey(str));
            BsonPlaceHolder valueHolder;
            BsonValueType valueType;
            db_dict_get_variant(fatPtr.ptr, &keyHolder, BsonKeyType.String, &valueType, &valueHolder);
            return SerializeObject.Internal_GetValue(valueType, ref valueHolder);

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
            db_dict_get(fatPtr.ptr, &str, BsonKeyType.String, BsonValueType.Int64, &v);
            return v;
        }
        public long GetInt(Guid guidKey)
        {
            long v;
            db_dict_get(fatPtr.ptr, &guidKey, BsonKeyType.Guid, BsonValueType.Int64, &v);
            return v;
        }
        public double GetFloat(Iterator ite)
        {
            double v;
            db_dict_ite_get(ite, &v, BsonValueType.Double);
            return v;
        }
        public double GetFloat(long intKey)
        {
            double v;
            db_dict_get(fatPtr.ptr, &intKey, BsonKeyType.Int64, BsonValueType.Double, &v);
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
            db_dict_get(fatPtr.ptr, &str, BsonKeyType.String, BsonValueType.Double, &v);
            return v;
        }
        public double GetFloat(Guid guidKey)
        {
            double v;
            db_dict_get(fatPtr.ptr, &guidKey, BsonKeyType.Guid, BsonValueType.Double, &v);
            return v;
        }
        public string GetString(Iterator ite)
        {
            CSString v;
            db_dict_ite_get(ite, &v, BsonValueType.String);
            if (v.bytes == null) return null; return new string((sbyte*)v.bytes, 0, (int)v.len);
        }
        public string GetKeyString(Iterator ite)
        {
            CSString v;
            db_dict_ite_getkey(ite, &v, BsonKeyType.String);

            if (v.bytes == null) return null; return new string((sbyte*)v.bytes, 0, (int)v.len);
        }
        public string GetString(long intKey)
        {
            CSString v;
            db_dict_get(fatPtr.ptr, &intKey, BsonKeyType.Int64, BsonValueType.String, &v);
            if (v.bytes == null) return null; return new string((sbyte*)v.bytes, 0, (int)v.len);
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
            db_dict_get(fatPtr.ptr, &str, BsonKeyType.String, BsonValueType.String, &v);
            if (v.bytes == null) return null; return new string((sbyte*)v.bytes, 0, (int)v.len);

        }
        public string GetString(Guid guidKey)
        {
            CSString v;
            db_dict_get(fatPtr.ptr, &guidKey, BsonKeyType.Guid, BsonValueType.String, &v);
            if (v.bytes == null) return null; return new string((sbyte*)v.bytes, 0, (int)v.len);

        }
        public Guid GetGuid(Iterator ite)
        {
            Guid v;
            db_dict_ite_get(ite, &v, BsonValueType.Guid);
            return v;
        }
        public Guid GetKeyGuid(Iterator ite)
        {
            Guid v;
            db_dict_ite_getkey(ite, &v, BsonKeyType.Guid);
            return v;
        }
        public Guid GetGuid(long intKey)
        {
            Guid v;
            db_dict_get(fatPtr.ptr, &intKey, BsonKeyType.Int64, BsonValueType.Guid, &v);
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
            db_dict_get(fatPtr.ptr, &str, BsonKeyType.String, BsonValueType.Guid, &v);
            return v;
        }
        public Guid GetGuid(Guid guidKey)
        {
            Guid v;
            db_dict_get(fatPtr.ptr, &guidKey, BsonKeyType.Guid, BsonValueType.Guid, &v);
            return v;
        }
        public SerializeArray GetArray(Iterator ite)
        {
            IntPtr v;
            db_dict_ite_get(ite, &v, BsonValueType.Array);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }

        public SerializeArray GetArray(long intKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &intKey, BsonKeyType.Int64, BsonValueType.Array, &v);
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
            db_dict_get(fatPtr.ptr, &str, BsonKeyType.String, BsonValueType.Array, &v);
            if (v.ToPointer() == null)
            {
                var x = v;
            }
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeArray GetArray(Guid guidKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &guidKey, BsonKeyType.Guid, BsonValueType.Array, &v);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }

        public SerializeDict GetDict(Iterator ite)
        {
            IntPtr v;
            db_dict_ite_get(ite, &v, BsonValueType.Dict);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeDict GetDict(long intKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &intKey, BsonKeyType.Int64, BsonValueType.Dict, &v);
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
            db_dict_get(fatPtr.ptr, &str, BsonKeyType.String, BsonValueType.Dict, &v);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeDict GetDict(Guid guidKey)
        {
            IntPtr v;
            db_dict_get(fatPtr.ptr, &guidKey, BsonKeyType.Guid, BsonValueType.Dict, &v);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }

        public void SetInt(long intKey, long value)
        {
            db_dict_set(fatPtr.ptr, &intKey, BsonKeyType.Int64, &value, BsonValueType.Int64);
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
            db_dict_set(fatPtr.ptr, &str, BsonKeyType.String, &value, BsonValueType.Int64);

        }
        public void SetInt(Guid guidKey, long value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, BsonKeyType.Guid, &value, BsonValueType.Int64);

        }
        public void SetFloat(long intKey, double value)
        {
            db_dict_set(fatPtr.ptr, &intKey, BsonKeyType.Int64, &value, BsonValueType.Double);
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
            db_dict_set(fatPtr.ptr, &str, BsonKeyType.String, &value, BsonValueType.Double);

        }
        public void SetFloat(Guid guidKey, double value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, BsonKeyType.Guid, &value, BsonValueType.Double);
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
            db_dict_set(fatPtr.ptr, &intKey, BsonKeyType.Int64, &strValue, BsonValueType.String);
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
            db_dict_set(fatPtr.ptr, &str, BsonKeyType.String, &strValue, BsonValueType.String);
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
            db_dict_set(fatPtr.ptr, &guidKey, BsonKeyType.Guid, &strValue, BsonValueType.String);
        }

        public void SetGuid(long intKey, Guid value)
        {
            db_dict_set(fatPtr.ptr, &intKey, BsonKeyType.Int64, &value, BsonValueType.Guid);
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
            db_dict_set(fatPtr.ptr, &str, BsonKeyType.String, &value, BsonValueType.Guid);

        }
        public void SetGuid(Guid guidKey, Guid value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, BsonKeyType.Guid, &value, BsonValueType.Guid);
        }

        public void SetArray(long intKey, SerializeArray.MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &intKey, BsonKeyType.Int64, &value, BsonValueType.Array);
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
            db_dict_set(fatPtr.ptr, &str, BsonKeyType.String, &value, BsonValueType.Array);
        }
        public void SetArray(Guid guidKey, SerializeArray.MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, BsonKeyType.Guid, &value, BsonValueType.Array);
        }
        public void SetDict(long intKey, MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &intKey, BsonKeyType.Int64, &value, BsonValueType.Dict);
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
            db_dict_set(fatPtr.ptr, &str, BsonKeyType.String, &value, BsonValueType.Dict);
        }
        public void SetDict(Guid guidKey, MovedHandle value)
        {
            db_dict_set(fatPtr.ptr, &guidKey, BsonKeyType.Guid, &value, BsonValueType.Dict);
        }

        public void Remove(long intKey)
        {
            db_dict_remove(fatPtr.ptr, &intKey, BsonKeyType.Int64);
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
            db_dict_remove(fatPtr.ptr, &str, BsonKeyType.String);
        }
        public void Remove(Guid guidKey)
        {
            db_dict_remove(fatPtr.ptr, &guidKey, BsonKeyType.Guid);
        }

    }
    public unsafe struct SerializeArray : IDisposable
    {
        public FatPtr fatPtr;
        public bool IsCreated
        {
            get { return fatPtr.ptr.ToPointer() != null; }
        }
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
        static extern void db_arr_get_value(IntPtr arr, int index, void* valuePtr, BsonValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_get_value_variant(IntPtr arr, int index, void* valuePtr, BsonValueType* valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_set_value(IntPtr arr, int index, void* valuePtr, BsonValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_add_value(IntPtr arr, void* valuePtr, BsonValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_remove(IntPtr arr, int index);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_itebegin(IntPtr dict, Iterator* ptr);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_iteend(IntPtr dict, Iterator* end, bool* result);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_ite_next(IntPtr dict, Iterator* end);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_ite_get(Iterator ite, void* valuePtr, BsonValueType valueType);
        [DllImport("VEngine_Database.dll")]
        static extern void db_arr_ite_get_variant(Iterator ite, void* valuePtr, BsonValueType* valueType);
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
            db_arr_get_value(fatPtr.ptr, index, &v, BsonValueType.Int64);
            return v;
        }
        public BsonValue Get(int index)
        {
            BsonPlaceHolder holder;
            BsonValueType type;
            db_arr_get_value_variant(fatPtr.ptr, index, &holder, &type);
            return SerializeObject.Internal_GetValue(type, ref holder);
        }
        public long GetInt(Iterator ite)
        {
            long v;
            db_arr_ite_get(ite, &v, BsonValueType.Int64);
            return v;
        }
        public BsonValue Get(Iterator ite)
        {
            BsonPlaceHolder holder;
            BsonValueType type;
            db_arr_ite_get_variant(ite, &holder, &type);
            return SerializeObject.Internal_GetValue(type, ref holder);
        }
        public string GetString(int index)
        {
            CSString v;
            db_arr_get_value(fatPtr.ptr, index, &v, BsonValueType.String);
            if (v.bytes == null) return null; return new string((sbyte*)v.bytes, 0, (int)v.len);
        }
        public string GetString(Iterator ite)
        {
            CSString v;
            db_arr_ite_get(ite, &v, BsonValueType.String);
            if (v.bytes == null) return null; return new string((sbyte*)v.bytes, 0, (int)v.len);
        }

        public double GetFloat(int index)
        {
            double v;
            db_arr_get_value(fatPtr.ptr, index, &v, BsonValueType.Double);
            return v;
        }
        public double GetFloat(Iterator ite)
        {
            double v;
            db_arr_ite_get(ite, &v, BsonValueType.Double);
            return v;
        }
        public SerializeDict GetDict(int index)
        {
            IntPtr v;
            db_arr_get_value(fatPtr.ptr, index, &v, BsonValueType.Dict);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeDict GetDict(Iterator ite)
        {
            IntPtr v;
            db_arr_ite_get(ite, &v, BsonValueType.Dict);
            return new SerializeDict(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeArray GetArray(int index)
        {
            IntPtr v;
            db_arr_get_value(fatPtr.ptr, index, &v, BsonValueType.Array);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }
        public SerializeArray GetArray(Iterator ite)
        {
            IntPtr v;
            db_arr_ite_get(ite, &v, BsonValueType.Array);
            return new SerializeArray(new FatPtr { ptr = v, isManaging = false });
        }
        public Guid GetGuid(int index)
        {
            Guid v;
            db_arr_get_value(fatPtr.ptr, index, &v, BsonValueType.Guid);
            return v;
        }
        public Guid GetGuid(Iterator ite)
        {
            Guid v;
            db_arr_ite_get(ite, &v, BsonValueType.Guid);
            return v;
        }

        public void AddInt(long value)
        {
            db_arr_add_value(fatPtr.ptr, &value, BsonValueType.Int64);
        }
        public void AddFloat(double value)
        {
            db_arr_add_value(fatPtr.ptr, &value, BsonValueType.Double);
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
            db_arr_add_value(fatPtr.ptr, &strValue, BsonValueType.String);
        }
        public void AddDict(SerializeDict.MovedHandle value)
        {
            db_arr_add_value(fatPtr.ptr, &value, BsonValueType.Dict);
        }
        public void AddArray(MovedHandle value)
        {
            db_arr_add_value(fatPtr.ptr, &value, BsonValueType.Array);
        }
        public void AddGuid(Guid value)
        {
            db_arr_add_value(fatPtr.ptr, &value, BsonValueType.Guid);
        }

        public void SetInt(int index, long value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, BsonValueType.Int64);
        }
        public void SetFloat(int index, double value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, BsonValueType.Double);
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
            db_arr_set_value(fatPtr.ptr, index, &strValue, BsonValueType.String);
        }
        public void SetDict(int index, SerializeDict.MovedHandle value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, BsonValueType.Dict);
        }
        public void SetArray(int index, MovedHandle value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, BsonValueType.Array);
        }
        public void SetGuid(int index, Guid value)
        {
            db_arr_set_value(fatPtr.ptr, index, &value, BsonValueType.Guid);
        }
        public void Remove(int index)
        {
            db_arr_remove(fatPtr.ptr, index);
        }
    }

}