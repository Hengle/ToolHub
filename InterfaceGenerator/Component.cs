using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
[GenerateCPP("VEngine_Unity.dll", "Component_codegen.hpp", "Component_codegen.cs", "toolhub", csharpPreDefine: "using Unity.Mathematics;\n")]
class Component
{
    public Component(CSharpString typeName, bool isValue) { }
    public Component(CSharpString typeName, CSharpString guid) { }
    public Component(IntPtr handle) { }
    public bool GetBool(CSharpString name) { return default; }
    public long GetInt(CSharpString name) { return default; }
    public double GetFloat(CSharpString name) { return default; }
    public CSharpString GetString(CSharpString name) { return default; }
    public IntPtr GetComponent(CSharpString name) { return default; }


    public BinaryArray GetBoolArray(CSharpString name) { return default; }
    public BinaryArray GetIntArray(CSharpString name) { return default; }
    public BinaryArray GetFloatArray(CSharpString name) { return default; }
    public BinaryArray GetComponentArray(CSharpString name) { return default; }

    public void SetInt(CSharpString name, long value) { }
    public void SetBool(CSharpString name, bool value) { }
    public void SetFloat(CSharpString name, double value) { }
    public void SetString(CSharpString name, CSharpString value) { }
    public void SetComponent(CSharpString name, IntPtr comp) { }
    public void SetIntArray(CSharpString name, BinaryArray value) { }
    public void SetFloatArray(CSharpString name, BinaryArray value) { }
    public void SetComponentArray(CSharpString name, BinaryArray value) { }
    public void Reset() { }
    public IntPtr GetHandle() { return default; }
    public ComponentType GetCompType() { return default; }
}

