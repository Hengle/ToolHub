using System;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;

[AttributeUsage(AttributeTargets.Class)]
public class GenerateCPP : Attribute
{
    public string cppPath { get; private set; }
    public string csharpPath { get; private set; }
    public string dllName { get; private set; }

    public GenerateCPP(
        string dllName,
        string cppPath,
        string csharpPath)
    {
        this.cppPath = cppPath;
        this.dllName = dllName;
        this.csharpPath = csharpPath;
    }
}
[GenerateCPP("VEngine_Unity.dll", "Test.hpp", "Test.cs")]
class TestClass
{
    public TestClass() { }
    public void RunData(int a, double b) { }
    public int Run1(int a, double b) { return new int(); }
}

class Program
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

    static void Main(string[] args)
    {
        var allTypes = GetTypesWithCPPAttri(Assembly.GetExecutingAssembly());
        {
            CPPPrinter printer = new CPPPrinter(false);
            foreach (var i in allTypes)
            {
                printer.Print("D:/ToolHub/UnityProject/Assets/", i.t, i.cpp);
            }
        }
        {
            CPPPrinter printer = new CPPPrinter(true);
            foreach (var i in allTypes)
            {
                printer.Print("D:/ToolHub/vengine/Unity/", i.t, i.cpp);
            }
        }
    }
}
