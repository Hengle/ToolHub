using System;
using System.Collections.Generic;
using System.Collections;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization.Formatters.Binary;

namespace Network
{
    public class VSerializable : Attribute
    {
        public VSerializable() { }

        public static void Serialize(
            object obj,
            BinaryFormatter formatter,
            Stream stream)
        {
            Type type = obj.GetType();
            if (type.GetCustomAttribute<VSerializable>() == null)
            {
                IList col = obj as IList;
                if (col != null)
                {
                    SerializeList(col, formatter, stream);
                    return;
                }
                formatter.Serialize(stream, obj);
            }
            else
            {
                var fields = type.GetFields(BindingFlags.Instance | BindingFlags.Public);
                foreach (var i in fields)
                {
                    object v = i.GetValue(obj);
                    Serialize(v, formatter, stream);
                }
            }
        }
        private static void SerializeList(
          in IList list,
          BinaryFormatter formatter,
          Stream stream)
        {
            long sz = list.Count;
            formatter.Serialize(stream, sz);
            foreach (var i in list)
            {
                Serialize(i, formatter, stream);
            }
        }

        public static object DeSerialize(
            Type type,
            BinaryFormatter formatter,
            Stream stream)
        {
            try
            {
                object obj = Activator.CreateInstance(type);
                if (type.GetCustomAttribute<VSerializable>() == null)
                {
                    IList col = obj as IList;
                    if (col != null)
                    {
                        DeSerializeList(col, formatter, stream);
                        return obj;
                    }
                    obj = formatter.Deserialize(stream);
                }
                else
                {
                    var fields = type.GetFields(BindingFlags.Instance | BindingFlags.Public);
                    foreach (var i in fields)
                    {
                        i.SetValue(obj, DeSerialize(i.FieldType, formatter, stream));
                    }
                }
                return obj;
            }catch(MissingMethodException exp)
            {
                return formatter.Deserialize(stream);
            }
        }
        private static void DeSerializeList(
            IList array,
            BinaryFormatter formatter,
            Stream stream)
        {
            array.Clear();
            Type t = array.GetType().GenericTypeArguments[0];
            long sz = (long)formatter.Deserialize(stream);
            for(long v = 0; v < sz; ++v)
            {
                array.Add(DeSerialize(t, formatter, stream));
            }
        }
    }

}