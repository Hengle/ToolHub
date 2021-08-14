using System;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;

class Codegen
{
    struct RefType
    {
        public Type t;
        public GenerateCPP cpp;
    }
    static IEnumerable<RefType> GetTypesWithCPPAttri(Assembly assembly)
    {
        foreach (Type type in assembly.GetTypes())
        {
            var t = type.GetCustomAttribute(typeof(GenerateCPP), true);
            if (t != null)
            {
                yield return new RefType
                {
                    t = type,
                    cpp = t as GenerateCPP
                };
            }
        }
    }
    static readonly string csharpStr = "D:/ToolHub/UnityProject/Assets/";
    static readonly string cppStr = "D:/ToolHub/vengine/Unity/";
    static void GenerateAll()
    {
        var allTypes = GetTypesWithCPPAttri(Assembly.GetExecutingAssembly());
        {
            CPPPrinter printer = new CPPPrinter(false);
            foreach (var i in allTypes)
            {
                printer.Print(csharpStr, i.t, i.cpp);
            }
        }
        {
            CPPPrinter printer = new CPPPrinter(true);
            foreach (var i in allTypes)
            {
                printer.Print(cppStr, i.t, i.cpp);
            }
        }
    }

    static void GenerateOne(Type t)
    {
        var ass = t.GetCustomAttribute(typeof(GenerateCPP), true);
        var rt = new RefType
        {
            t = t,
            cpp = ass as GenerateCPP
        };
        {
            CPPPrinter printer = new CPPPrinter(false);
            printer.Print(csharpStr, rt.t, rt.cpp);

        }
        {
            CPPPrinter printer = new CPPPrinter(true);
            printer.Print(cppStr, rt.t, rt.cpp);

        }
    }
}