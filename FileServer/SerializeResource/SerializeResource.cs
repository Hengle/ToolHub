using System;
using System.Collections;
using System.Collections.Generic;
using MongoDB.Bson;
using MongoDB.Driver;
using Network;
namespace FileServer
{
    public unsafe class SerializeResource : IDisposable
    {
        public vstd.SerializeObject serObj { get; private set; }
        string path;
        public vstd.Guid rootGuid;
        public SerializeResource(string path, bool createNew, in vstd.Guid rootGuid)
        {
            serObj = new vstd.SerializeObject(true);
            this.path = path;
            this.rootGuid = rootGuid;
            if (!createNew)
                serObj.ReadFromFile(path);
        }
        private SerializeMember GetMember(vstd.SerializeDict memberDict)
        {
            SerializeValueType valueType;
            {
                var valueTypeInt = memberDict.GetInt("type");
                if (valueTypeInt == 0 || valueTypeInt >= (long)SerializeValueType.Num)
                    return new SerializeMember
                    {
                        type = SerializeValueType.None,
                        isArray = false,
                        value = null
                    };
                valueType = (SerializeValueType)valueTypeInt;
            }
            bool isArray = memberDict.GetInt("is_array") != 0;
            SerializeMember member = new SerializeMember
            {
                type = valueType,
                isArray = isArray,
                value = null
            };
            switch (valueType)
            {
                case SerializeValueType.Reference:
                case SerializeValueType.Structure:
                    if (isArray)
                    {
                        var arr = memberDict.GetArray("value");
                        if (!arr.IsCreated) return member;
                        List<vstd.Guid> guidList = new List<vstd.Guid>(arr.Length);
                        for (var v = arr.Begin(); !arr.End(v); arr.GetNext(ref v))
                        {
                            guidList.Add(arr.GetGuid(v));
                        }
                        member.value = guidList;
                    }
                    else
                    {
                        member.value = memberDict.GetGuid("value");
                    }
                    break;
                case SerializeValueType.String:
                    if (isArray)
                    {
                        var arr = memberDict.GetArray("value");
                        if (!arr.IsCreated) return member;
                        List<string> strList = new List<string>(arr.Length);
                        for (var v = arr.Begin(); !arr.End(v); arr.GetNext(ref v))
                        {
                            strList.Add(arr.GetString(v));
                        }
                        member.value = strList;
                    }
                    else
                    {
                        member.value = memberDict.GetString("value");
                    }
                    break;
                case SerializeValueType.Bool:
                    if (isArray)
                    {
                        var arr = memberDict.GetArray("value");
                        if (!arr.IsCreated) return member;
                        List<bool> strList = new List<bool>(arr.Length);
                        for (var v = arr.Begin(); !arr.End(v); arr.GetNext(ref v))
                        {
                            strList.Add(arr.GetInt(v) != 0);
                        }
                        member.value = strList;
                    }
                    else
                    {
                        member.value = memberDict.GetInt("value") != 0;
                    }
                    break;
                case SerializeValueType.Int:
                    if (isArray)
                    {
                        var arr = memberDict.GetArray("value");
                        if (!arr.IsCreated) return member;
                        List<long> strList = new List<long>(arr.Length);
                        for (var v = arr.Begin(); !arr.End(v); arr.GetNext(ref v))
                        {
                            strList.Add(arr.GetInt(v));
                        }
                        member.value = strList;
                    }
                    else
                    {
                        member.value = memberDict.GetInt("value");
                    }
                    break;
                case SerializeValueType.Float:
                    if (isArray)
                    {
                        var arr = memberDict.GetArray("value");
                        if (!arr.IsCreated) return member;
                        List<double> strList = new List<double>(arr.Length);
                        for (var v = arr.Begin(); !arr.End(v); arr.GetNext(ref v))
                        {
                            strList.Add(arr.GetFloat(v));
                        }
                        member.value = strList;
                    }
                    else
                    {
                        member.value = memberDict.GetFloat("value");
                    }
                    break;
            }
            return member;
        }
        private SerializeStruct GetStruct(vstd.SerializeDict dict)
        {
            SerializeStruct result = new SerializeStruct
            {
                members = new Dictionary<string, SerializeMember>(dict.Length)
            };
            for (var memIte = dict.Begin(); !dict.End(memIte); dict.GetNext(ref memIte))
            {
                string s = dict.GetKeyString(memIte);
                var memberDict = dict.GetDict(memIte);
                if (s == null || !memberDict.IsCreated) continue;

                result.members.Add(s, GetMember(memberDict));
            }
            return result;
        }
        public SerializeStruct GetStruct(in vstd.Guid id)
        {
            var root = serObj.GetRootNode();
            vstd.SerializeDict dict = root.GetDict(id);
            if (!dict.IsCreated)
            {
                return new SerializeStruct();
            }
            return GetStruct(dict);
        }

        private void SetValueToDict(
            vstd.SerializeDict dict,
            string name,
            in SerializeMember v)
        {
            switch (v.type)
            {
                case SerializeValueType.Structure:
                case SerializeValueType.Reference:
                    if (v.isArray)
                    {
                        var arr = serObj.CreateNewArray();
                        List<vstd.Guid> guidList = (List<vstd.Guid>)v.value;
                        foreach (var i in guidList)
                        {
                            arr.AddGuid(i);
                        }
                        dict.SetArray(name, arr.Move());
                    }
                    else
                    {
                        dict.SetGuid(name, (vstd.Guid)v.value);
                    }
                    break;
                case SerializeValueType.String:
                    if (v.isArray)
                    {
                        var arr = serObj.CreateNewArray();
                        List<string> guidList = (List<string>)v.value;
                        foreach (var i in guidList)
                        {
                            arr.AddString(i);
                        }
                        dict.SetArray(name, arr.Move());
                    }
                    else
                    {
                        dict.SetString(name, (string)v.value);
                    }
                    break;

                case SerializeValueType.Bool:
                    if (v.isArray)
                    {
                        var arr = serObj.CreateNewArray();
                        List<bool> guidList = (List<bool>)v.value;
                        foreach (var i in guidList)
                        {
                            arr.AddInt(i ? 1 : 0);
                        }
                        dict.SetArray(name, arr.Move());
                    }
                    else
                    {
                        dict.SetInt(name, (bool)v.value ? 1 : 0);
                    }
                    break;
                case SerializeValueType.Int:
                    if (v.isArray)
                    {
                        var arr = serObj.CreateNewArray();
                        List<long> guidList = (List<long>)v.value;
                        foreach (var i in guidList)
                        {
                            arr.AddInt(i);
                        }
                        dict.SetArray(name, arr.Move());
                    }
                    else
                    {
                        dict.SetInt(name, (long)v.value);
                    }
                    break;
                case SerializeValueType.Float:
                    if (v.isArray)
                    {
                        var arr = serObj.CreateNewArray();
                        List<double> guidList = (List<double>)v.value;
                        foreach (var i in guidList)
                        {
                            arr.AddFloat(i);
                        }
                        dict.SetArray(name, arr.Move());
                    }
                    else
                    {
                        dict.SetFloat(name, (double)v.value);
                    }
                    break;
            }
        }
        private void SetStruct(vstd.SerializeDict dict, SerializeStruct st)
        {
            foreach (var kv in st.members)
            {
                var v = kv.Value;
                SetValueToDict(dict, kv.Key, v);
            }
        }
        public void SetStruct(in vstd.Guid id, SerializeStruct st)
        {
            var root = serObj.GetRootNode();
            RemoveStruct(id);
            if (st.members == null)
            {
                return;
            }
            var dict = serObj.CreateNewDict();
            root.SetDict(id, dict.Move());
            SetStruct(dict, st);
        }

        private void RemoveStruct(vstd.SerializeDict dict)
        {

            for (var memIte = dict.Begin(); !dict.End(memIte); dict.GetNext(ref memIte))
            {
                string s = dict.GetKeyString(memIte);
                var memberDict = dict.GetDict(memIte);
                if (s == null || !memberDict.IsCreated) continue;
                SerializeValueType valueType;
                {
                    var valueTypeInt = memberDict.GetInt("type");
                    if (valueTypeInt == 0 || valueTypeInt >= (long)SerializeValueType.Num)
                        continue;
                    valueType = (SerializeValueType)valueTypeInt;
                }
                if (valueType != SerializeValueType.Structure)
                    continue;
                bool isArray = memberDict.GetInt("is_array") != 0;
                if (isArray)
                {
                    var valueArr = memberDict.GetArray("value");
                    if (!valueArr.IsCreated) continue;
                    for (var i = valueArr.Begin(); !valueArr.End(i); valueArr.GetNext(ref i))
                    {
                        RemoveStruct(valueArr.GetGuid(i));
                    }
                }
                else
                {
                    var valueArr = memberDict.GetGuid("value");
                    RemoveStruct(valueArr);
                }
            }
        }
        public void RemoveStruct(in vstd.Guid id)
        {
            var root = serObj.GetRootNode();
            vstd.SerializeDict dict = root.GetDict(id);
            if (!dict.IsCreated)
            {
                return;
            }
            RemoveStruct(dict);
            root.Remove(id);
        }
        public void SetValue(in vstd.Guid id, string name, in SerializeMember member)
        {
            var root = serObj.GetRootNode();
            vstd.SerializeDict dict = root.GetDict(id);
            if (!dict.IsCreated)
            {
                return;
            }
            var memDict = dict.GetDict(name);
            long value = memDict.GetInt("type");
            if (value == (long)SerializeValueType.Structure)
            {
                bool isArray = memDict.GetInt("is_array") != 0;
                if (isArray)
                {
                    var arr = memDict.GetArray("value");
                    for (var ite = arr.Begin(); !arr.End(ite); arr.GetNext(ref ite))
                    {
                        root.Remove(arr.GetGuid(ite));
                    }
                }
                else
                {
                    root.Remove(memDict.GetGuid("value"));
                }
            }
            SetValueToDict(dict, name, member);
        }
        public SerializeMember GetValue(in vstd.Guid id, string name)
        {

            var root = serObj.GetRootNode();
            vstd.SerializeDict dict = root.GetDict(id);
            if (!dict.IsCreated)
            {
                return new SerializeMember
                {
                    type = SerializeValueType.None
                };
            }
            return GetMember(dict);
        }
        public void Dispose()
        {
            if (serObj.IsCreated)
            {
                serObj.SerializeToFile(path);
                serObj.Dispose();
            }
        }
    }
}