using System.Reflection;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;
using System.Threading;
namespace Network
{

    public unsafe static class RPCReflector
    {
        struct AttributeResult<T> where T : Attribute
        {
            public Type type;
            public T attribute;
        }
        static IEnumerable<AttributeResult<T>> GetTypesWithAttri<T>(Assembly assembly) where T : Attribute
        {
            foreach (Type type in assembly.GetTypes())
            {
                var t = type.GetCustomAttribute(typeof(T), true);
                if (t != null)
                {
                    yield return new AttributeResult<T>
                    {
                        type = type,
                        attribute = t as T
                    };
                }
            }
        }
        static Dictionary<string, Action<Stream, BinaryFormatter>> executableFuncs = new Dictionary<string, Action<Stream, BinaryFormatter>>();
        static object[] emptyArr = new object[] { null };
        static ThreadLocal<byte[]> bytesArr = new ThreadLocal<byte[]>();
        static byte[] TLocalBytesArray
        {
            get
            {
                if (bytesArr.Value == null)
                {
                    bytesArr.Value = new byte[4096];
                }
                return bytesArr.Value;
            }
        }
        public static void LoadRPCFunctor(Assembly assembly, RPCLayer layer)
        {
            void MakeMethods(MethodInfo method, Type clsType)
            {
                var pars = method.GetParameters();
                if (method.ReturnType != typeof(void))
                {
                    throw new FormatException("Currently RPC do not support return value: " + clsType.Name + "::" + method.Name);
                }
                Action<Stream, BinaryFormatter> callable = (stream, fmt) =>
                {
                    byte[] bytesArr = TLocalBytesArray;
                    stream.Read(bytesArr, 0, 1);
                    byte isObjectContained = bytesArr[0];
                    if (isObjectContained == 0)
                    {
                        if (pars.Length == 0)
                            method.Invoke(null, null);
                        else
                            method.Invoke(null, emptyArr);
                    }
                    else if (isObjectContained == 1)
                    {
                        var par = VSerializable.DeSerialize(pars[0].ParameterType, fmt, stream);
                        method.Invoke(null, new object[] { par });
                    }
                    else
                    {
                        object[] objs = new object[isObjectContained];
                        for (int i = 0; i < objs.Length; ++i)
                        {
                            objs[i] = VSerializable.DeSerialize(pars[i].ParameterType, fmt, stream);
                        }
                        method.Invoke(null, objs);

                    }
                };
                string name = clsType.Name + '#' + method.Name;
                lock (executableFuncs)
                {
                    executableFuncs.Add(name, callable);
                }
            }

            foreach (var i in GetTypesWithAttri<RPCAttribute>(assembly))
            {
                if ((i.attribute.layer & layer) == 0) continue;
                MethodInfo[] infos = i.type.GetMethods(BindingFlags.Static | BindingFlags.Public);
                foreach (var m in infos)
                {
                    MakeMethods(m, i.type);
                }
            }
        }

        public static void CallFunc(
            Stream stream,
            BinaryFormatter formatter,
            string className,
            string funcName,
            object arg)
        {
            int strSize = 4 + className.Length + funcName.Length + 2;
            byte[] arr = new byte[strSize];
            fixed (byte* originPtr = arr)
            {
                byte* ptr = originPtr;
                *(int*)ptr = className.Length + funcName.Length + 1;
                ptr += 4;
                foreach (char i in className)
                {
                    *ptr = (byte)i;
                    ptr++;
                }
                *ptr = (byte)'#';
                ptr++;
                foreach (char i in funcName)
                {
                    *ptr = (byte)i;
                    ptr++;
                }

                if (arg == null)
                {
                    *ptr = 0;
                    stream.Write(arr, 0, arr.Length);
                }
                else
                {
                    *ptr = 1;
                    stream.Write(arr, 0, arr.Length);
                    VSerializable.Serialize(arg, formatter, stream);

                }
            }

        }
        public static void CallFunc(
           Stream stream,
           BinaryFormatter formatter,
           string className,
           string funcName,
           object[] arg)
        {
            int strSize = 4 + className.Length + funcName.Length + 2;
            byte[] arr = new byte[strSize];
            fixed (byte* originPtr = arr)
            {
                byte* ptr = originPtr;
                *(int*)ptr = className.Length + funcName.Length + 1;
                ptr += 4;
                foreach (char i in className)
                {
                    *ptr = (byte)i;
                    ptr++;
                }
                *ptr = (byte)'#';
                ptr++;
                foreach (char i in funcName)
                {
                    *ptr = (byte)i;
                    ptr++;
                }

                if (arg == null)
                {
                    *ptr = 0;
                    stream.Write(arr, 0, arr.Length);
                }
                else
                {
                    *ptr = (byte)arg.Length;
                    stream.Write(arr, 0, arr.Length);
                    foreach (var i in arg)
                        VSerializable.Serialize(i, formatter, stream);

                }
            }

        }
        public static bool ExecuteStream(
            Stream stream,
            BinaryFormatter formatter)
        {
            byte[] byteArr = TLocalBytesArray;
            stream.Read(byteArr, 0, sizeof(int));
            int strLen;
            fixed (byte* ptr = byteArr)
            {
                strLen = *(int*)ptr;
            }
            if (strLen == 0)
                return false;
            fixed (byte* bPtr = byteArr)
            {
                sbyte* funcNamePtr = (sbyte*)bPtr;
                stream.Read(byteArr, 0, strLen);
                string funcName = new string(funcNamePtr, 0, strLen);
                lock (executableFuncs)
                {
                    Action<Stream, BinaryFormatter> func;
                    if (executableFuncs.TryGetValue(funcName, out func))
                    {
                        func(stream, formatter);
                    }
                    else
                    {
                        UnityEngine.Debug.Log("Erorr: Try call non-exists function " + funcName);
                        throw new Exception();
                    }
                }
            }
            return true;

        }
    }
}