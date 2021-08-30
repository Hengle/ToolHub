using System.Reflection;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;
using Native;
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
                    byte isObjectContained = 0;
                    stream.Read(new Span<byte>(&isObjectContained, 1));
                    if (isObjectContained == 0)
                    {
                        if (pars.Length == 0)
                            method.Invoke(null, null);
                        else
                            method.Invoke(null, emptyArr);
                    }
                    else if (isObjectContained == 1)
                    {
                        var par = fmt.Deserialize(stream);
                        method.Invoke(null, new object[] { par });
                    }
                    else
                    {
                        object[] objs = new object[isObjectContained];
                        for (int i = 0; i < objs.Length; ++i)
                        {
                            objs[i] = fmt.Deserialize(stream);
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
            byte* ptr = stackalloc byte[strSize];
            *(int*)ptr = className.Length + funcName.Length + 1;
            ReadOnlySpan<byte> sp = new ReadOnlySpan<byte>(ptr, strSize);
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
                stream.Write(sp);
            }
            else
            {
                *ptr = 1;
                stream.Write(sp);
                formatter.Serialize(stream, arg);
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
            byte* ptr = stackalloc byte[strSize];
            *(int*)ptr = className.Length + funcName.Length + 1;
            ReadOnlySpan<byte> sp = new ReadOnlySpan<byte>(ptr, strSize);
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
                stream.Write(sp);
            }
            else
            {
                *ptr = (byte)arg.Length;
                stream.Write(sp);
                foreach (var i in arg)
                    formatter.Serialize(stream, i);
            }

        }
        public static void ExecuteStream(
            Stream stream,
            BinaryFormatter formatter)
        {
            int strLen = 0;
            Span<byte> lenSpan = new Span<byte>((byte*)&strLen, 4);
            stream.Read(lenSpan);
            if (strLen == 0) return;
            sbyte* funcNamePtr = stackalloc sbyte[strLen];
            stream.Read(new Span<byte>(funcNamePtr, strLen));
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
                    Console.WriteLine("Erorr: Try call non-exists function " + funcName);
                    throw new Exception();
                }
            }
        }
    }
}