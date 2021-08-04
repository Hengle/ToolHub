using System;
using System.Collections.Generic;
using System.Collections;
using System.Reflection;
using System.IO;


[AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct)]
public class GenerateCPP : Attribute
{
    public string cppPath { get; private set; }
    public string csharpPath { get; private set; }
    public string dllName { get; private set; }
    public string nameSpace { get; private set; }
    public string cppPreDefine { get; private set; }
    public string csharpPreDefine { get; private set; }

    public GenerateCPP(
        string dllName,
        string cppPath,
        string csharpPath,
        string nameSpace = "",
        string cppPreDefine = "",
        string csharpPreDefine = "")
    {
        this.cppPath = cppPath;
        this.dllName = dllName;
        this.csharpPath = csharpPath;
        this.nameSpace = nameSpace;
        this.cppPreDefine = cppPreDefine;
        this.csharpPreDefine = csharpPreDefine;
    }
}
class CPPPrinter
{
    Dictionary<Type, string> typeName;
    Dictionary<string, bool> ignoreInfos;
    bool isCPP;
    public CPPPrinter(bool isCPP)
    {
        this.isCPP = isCPP;
        typeName = new Dictionary<Type, string>();
        if (isCPP)
        {
            typeName.Add(typeof(byte), "uint8_t");
            typeName.Add(typeof(sbyte), "int8_t");
            typeName.Add(typeof(char), "wchar_t");
            typeName.Add(typeof(short), "int16_t");
            typeName.Add(typeof(ushort), "uint16_t");
            typeName.Add(typeof(int), "int32");
            typeName.Add(typeof(long), "int64");
            typeName.Add(typeof(ulong), "uint64");
            typeName.Add(typeof(IntPtr), "void*");
            typeName.Add(typeof(UIntPtr), "void*");

        }
        else
        {
            typeName.Add(typeof(byte), "byte");
            typeName.Add(typeof(sbyte), "sbyte");
            typeName.Add(typeof(char), "char");
            typeName.Add(typeof(short), "short");
            typeName.Add(typeof(ushort), "ushort");
            typeName.Add(typeof(int), "int");
            typeName.Add(typeof(long), "long");
            typeName.Add(typeof(ulong), "ulong");
            typeName.Add(typeof(IntPtr), "IntPtr");
            typeName.Add(typeof(UIntPtr), "UIntPtr");
        }
        typeName.Add(typeof(uint), "uint");
        typeName.Add(typeof(void), "void");
        typeName.Add(typeof(bool), "bool");
        typeName.Add(typeof(float), "float");
        typeName.Add(typeof(double), "double");
        ignoreInfos = new Dictionary<string, bool>();
        var objMethods = typeof(object).GetMethods();
        foreach (var i in objMethods)
        {
            ignoreInfos.TryAdd(i.Name, true);
        }
    }
    string GetName(Type t)
    {
        string r;
        if (typeName.TryGetValue(t, out r)) return r;
        return t.Name;
    }
    string GetClassFunc(MethodInfo mtd, bool useRef)
    {
        string result = GetName(mtd.ReturnType) + " "
             + mtd.Name + '(';

        var ps = mtd.GetParameters();
        if (useRef)
            result += GetParametersRef(ps, false);
        else
            result += GetParameters(ps, false);
        result += ")";
        return result;
    }
    string GetCPPClassFuncImpl(Type clsType, MethodInfo mtd)
    {
        string result = GetName(mtd.ReturnType) + " "
            + clsType.Name + "::" + mtd.Name + '(';

        var ps = mtd.GetParameters();
        result += GetParametersRef(ps, false);
        result += "){\n}";
        return result;
    }
    static readonly string retValue = "F32D2BF6260A4FBD";
    static readonly string thsValue = "AFD920F282E74FF8";
    string GetCPPClassExternFunc(string spacedName, Type clsType, MethodInfo mtd, out bool isRet)
    {
        string result = "VENGINE_UNITY_EXTERN void " + spacedName + '_' + mtd.Name
            + '(';
        isRet = mtd.ReturnType != typeof(void);
        if (isRet) result += GetName(mtd.ReturnType) + "& " + retValue + ',';
        result += GetName(clsType) + "* " + thsValue + GetParameters(mtd.GetParameters(), true) + ")";
        return result;
    }
    IEnumerable<MethodInfo> GetMethods(IEnumerable<MethodInfo> infos)
    {
        foreach (var i in infos)
        {
            if (!ignoreInfos.ContainsKey(i.Name))
            {
                yield return i;
            }
        }
    }
    string GetParameters(ParameterInfo[] para, bool alreadyHaveArgs)
    {
        string result = "";
        if (para.Length > 0)
        {
            if (alreadyHaveArgs)
            {
                result += ',';
            }
            for (int i = 0; i < para.Length - 1; ++i)
            {
                result += GetName(para[i].ParameterType) + ' ' + para[i].Name + ',';
            }
            result += GetName(para[para.Length - 1].ParameterType) + ' ' + para[para.Length - 1].Name;
        }
        return result;
    }
    string GetParametersRef(ParameterInfo[] para, bool alreadyHaveArgs)
    {
        string result = "";
        if (para.Length > 0)
        {
            if (alreadyHaveArgs)
            {
                result += ',';
            }
            for (int i = 0; i < para.Length - 1; ++i)
            {
                result += GetName(para[i].ParameterType) + "& " + para[i].Name + ',';
            }
            result += GetName(para[para.Length - 1].ParameterType) + "& " + para[para.Length - 1].Name;
        }
        return result;
    }
    string GetTypelessParameters(ParameterInfo[] para, bool alreadyHaveArgs)
    {
        string result = "";
        if (para.Length > 0)
        {
            if (alreadyHaveArgs)
            {
                result += ',';
            }
            for (int i = 0; i < para.Length - 1; ++i)
            {
                result += para[i].Name + ',';
            }
            result += para[para.Length - 1].Name;
        }
        return result;
    }
    string GetCPPClassConstructor(string name, ConstructorInfo info)
    {
        string result = name + '(' + GetParametersRef(info.GetParameters(), false) + ");";
        return result;
    }
    string GetCPPExternConstructor(string name, string spaceName, ConstructorInfo info, int index)
    {
        string result = "VENGINE_UNITY_EXTERN " + name + "* Create_" + spaceName + index.ToString() + '(' + GetParameters(info.GetParameters(), false) + "){\nreturn new "
            + name + '(' + GetTypelessParameters(info.GetParameters(), false) + ");\n}";
        return result;
    }
    public string PrintCPPClass(Type clsType, GenerateCPP attri)
    {

        var clsName = GetName(clsType);
        var spacedClsName = attri.nameSpace + '_' + clsName;
        string result = "#pragma once\n" +
            "#include <Unity/UnityInclude.h>\n";
        result += attri.cppPreDefine;
        void PrintClass()
        {
            result += "class " + clsName + " final : public vstd::IOperatorNewBase {\npublic:\n";
            var cons = clsType.GetConstructors(BindingFlags.Public | BindingFlags.Instance);
            foreach (var i in cons)
            {
                result += GetCPPClassConstructor(clsName, i) + '\n';
            }

            var mtds = GetMethods(clsType.GetMethods(BindingFlags.Public | BindingFlags.Instance));
            foreach (var i in mtds)
            {
                result += GetClassFunc(i, true)
                 + ";\n";
            }
            result += "};\n";
            //Extern Func
            foreach (var i in mtds)
            {
                bool isRet;
                result += GetCPPClassExternFunc(spacedClsName, clsType, i, out isRet);
                result += "{\n";
                if (isRet)
                {
                    result += retValue + "=";
                }
                result += thsValue + "->" + i.Name + "(";
                var pars = i.GetParameters();
                if (pars.Length > 0)
                {
                    for (int v = 0; v < (pars.Length - 1); ++v)
                    {
                        result += pars[v].Name + ",";
                    }
                    result += pars[pars.Length - 1].Name;
                }
                result += ");\n}\n";
            }
            result += "VENGINE_UNITY_EXTERN void Dispose_" + spacedClsName + '(' + clsName + "* v){\ndelete v;\n}\n";
            {
                int conIndex = 0;
                foreach (var i in cons)
                {
                    result += GetCPPExternConstructor(clsName, spacedClsName, i, conIndex) + '\n';
                    conIndex++;
                }
            }
            //Comments
            result += "/*\n";
            foreach (var i in cons)
            {
                result += clsName + "::" + clsName + '(' + GetParametersRef(i.GetParameters(), false) + "){\n}\n";
            }
            foreach (var i in mtds)
            {
                result += GetCPPClassFuncImpl(clsType, i);
                result += '\n';
            }
            result += "*/\n";
        }
        if (!string.IsNullOrEmpty(attri.nameSpace))
        {
            result += "namespace " + attri.nameSpace + "{\n";
            PrintClass();
            result += "\n}\n";
        }
        else
        {
            PrintClass();
        }
        return result;
    }

    public string PrintCSharpClass(Type clsType, GenerateCPP attri)
    {
        var clsName = GetName(clsType);
        var spacedClsName = attri.nameSpace + '_' + clsName;
        string result = attri.csharpPreDefine + "using System.Runtime.InteropServices;\n"
             + "using System;\n"
             + "using System.Collections.Generic;\n"
             + "using System.Collections;\n"
             + "using MPipeline;\n";
        void PrintClass()
        {
            result += "unsafe " + (clsType.IsClass ? "class" : "struct") + ' ' + clsName + ": IDisposable{\n";
            var mtds = GetMethods(clsType.GetMethods(BindingFlags.Public | BindingFlags.Instance));
            //Internal Methods
            foreach (var i in mtds)
            {

                result += "public " + GetClassFunc(i, false) + "{\n";
                if (i.ReturnType == typeof(void))
                {
                    result += spacedClsName + '_' + i.Name + "(instHandle" + GetTypelessParameters(i.GetParameters(), true) + ");";
                }
                else
                {
                    result += GetName(i.ReturnType) + " v=default;\n";
                    result += spacedClsName + '_' + i.Name + "(ref v,instHandle" + GetTypelessParameters(i.GetParameters(), true) + ");\n";
                    result += "return v;";
                }
                result += "\n}\n";
            }
            //Extern Methods
            foreach (var i in mtds)
            {
                result += "[DllImport(\"" + attri.dllName + "\")]\nstatic extern void " + spacedClsName + '_' + i.Name + '(';
                if (i.ReturnType != typeof(void))
                {
                    result += "ref " + GetName(i.ReturnType) + ' ' + retValue + ',';
                }
                result += "IntPtr " + thsValue + GetParameters(i.GetParameters(), true) + ");\n";
            }
            var cons = clsType.GetConstructors(BindingFlags.Public | BindingFlags.Instance);
            result += "private IntPtr instHandle;\n";
            //Internal Constructor
            int index = 0;
            foreach (var i in cons)
            {
                result += "public " + clsName + '(' + GetParameters(i.GetParameters(), false) + "){\n"
                    + "instHandle=Create_" + spacedClsName + index.ToString() + '(' + GetTypelessParameters(i.GetParameters(), false) + ");\n}\n";
                index++;
            }
            //Extern Constructor
            index = 0;
            foreach (var i in cons)
            {
                result += "[DllImport(\"" + attri.dllName + "\")]\nstatic extern IntPtr Create_" + spacedClsName + index.ToString() + '(' + GetParameters(i.GetParameters(), false) + ");\n";
                index++;
            }
            //Internal Disposer:
            result += "public void Dispose(){\nDispose_" + spacedClsName + "(instHandle);\n}\n";
            //Extern Disposer:
            result += "[DllImport(\"" + attri.dllName + "\")]\nstatic extern void Dispose_" + spacedClsName + "(IntPtr h);\n";
            result += "}\n";
        }
        if (!string.IsNullOrEmpty(attri.nameSpace))
        {
            result += "namespace " + attri.nameSpace + "{\n";
            PrintClass();
            result += "\n}\n";
        }
        else
        {
            PrintClass();
        }
        return result;
    }
    public void Print(string path, Type t, GenerateCPP attri)
    {
        if (isCPP)
            File.WriteAllText(path + attri.cppPath, PrintCPPClass(t, attri));
        else
            File.WriteAllText(path + attri.csharpPath, PrintCSharpClass(t, attri));
    }
}

