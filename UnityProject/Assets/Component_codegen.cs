using Unity.Mathematics;
using System.Runtime.InteropServices;
using System;
using System.Collections.Generic;
using System.Collections;
using MPipeline;
namespace toolhub{
unsafe class Component: IDisposable{
public bool GetBool(CSharpString name){
bool v=default;
toolhub_Component_GetBool(ref v,instHandle,name);
return v;
}
public long GetInt(CSharpString name){
long v=default;
toolhub_Component_GetInt(ref v,instHandle,name);
return v;
}
public double GetFloat(CSharpString name){
double v=default;
toolhub_Component_GetFloat(ref v,instHandle,name);
return v;
}
public CSharpString GetString(CSharpString name){
CSharpString v=default;
toolhub_Component_GetString(ref v,instHandle,name);
return v;
}
public IntPtr GetComponent(CSharpString name){
IntPtr v=default;
toolhub_Component_GetComponent(ref v,instHandle,name);
return v;
}
public BinaryArray GetBoolArray(CSharpString name){
BinaryArray v=default;
toolhub_Component_GetBoolArray(ref v,instHandle,name);
return v;
}
public BinaryArray GetIntArray(CSharpString name){
BinaryArray v=default;
toolhub_Component_GetIntArray(ref v,instHandle,name);
return v;
}
public BinaryArray GetFloatArray(CSharpString name){
BinaryArray v=default;
toolhub_Component_GetFloatArray(ref v,instHandle,name);
return v;
}
public BinaryArray GetComponentArray(CSharpString name){
BinaryArray v=default;
toolhub_Component_GetComponentArray(ref v,instHandle,name);
return v;
}
public void SetInt(CSharpString name,long value){
toolhub_Component_SetInt(instHandle,name,value);
}
public void SetBool(CSharpString name,bool value){
toolhub_Component_SetBool(instHandle,name,value);
}
public void SetFloat(CSharpString name,double value){
toolhub_Component_SetFloat(instHandle,name,value);
}
public void SetString(CSharpString name,CSharpString value){
toolhub_Component_SetString(instHandle,name,value);
}
public void SetComponent(CSharpString name,IntPtr comp){
toolhub_Component_SetComponent(instHandle,name,comp);
}
public void SetIntArray(CSharpString name,BinaryArray value){
toolhub_Component_SetIntArray(instHandle,name,value);
}
public void SetFloatArray(CSharpString name,BinaryArray value){
toolhub_Component_SetFloatArray(instHandle,name,value);
}
public void SetComponentArray(CSharpString name,BinaryArray value){
toolhub_Component_SetComponentArray(instHandle,name,value);
}
public void SetBoolArray(CSharpString name,BinaryArray value){
toolhub_Component_SetBoolArray(instHandle,name,value);
}
public void Reset(){
toolhub_Component_Reset(instHandle);
}
public IntPtr GetHandle(){
IntPtr v=default;
toolhub_Component_GetHandle(ref v,instHandle);
return v;
}
public bool IsValueType(){
bool v=default;
toolhub_Component_IsValueType(ref v,instHandle);
return v;
}
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetBool(ref bool F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetInt(ref long F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetFloat(ref double F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetString(ref CSharpString F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetComponent(ref IntPtr F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetBoolArray(ref BinaryArray F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetIntArray(ref BinaryArray F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetFloatArray(ref BinaryArray F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetComponentArray(ref BinaryArray F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,CSharpString name);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetInt(IntPtr AFD920F282E74FF8,CSharpString name,long value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetBool(IntPtr AFD920F282E74FF8,CSharpString name,bool value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetFloat(IntPtr AFD920F282E74FF8,CSharpString name,double value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetString(IntPtr AFD920F282E74FF8,CSharpString name,CSharpString value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetComponent(IntPtr AFD920F282E74FF8,CSharpString name,IntPtr comp);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetIntArray(IntPtr AFD920F282E74FF8,CSharpString name,BinaryArray value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetFloatArray(IntPtr AFD920F282E74FF8,CSharpString name,BinaryArray value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetComponentArray(IntPtr AFD920F282E74FF8,CSharpString name,BinaryArray value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_SetBoolArray(IntPtr AFD920F282E74FF8,CSharpString name,BinaryArray value);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_Reset(IntPtr AFD920F282E74FF8);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_GetHandle(ref IntPtr F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Component_IsValueType(ref bool F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8);
public IntPtr instHandle{get; private set;}
public Component(IntPtr newHandle){
instHandle = newHandle;
}
public Component(CSharpString typeName,IntPtr parentDatabase){
instHandle=Create_toolhub_Component0(typeName,parentDatabase);
}
[DllImport("VEngine_Unity.dll")]
static extern IntPtr Create_toolhub_Component0(CSharpString typeName,IntPtr parentDatabase);
public void Dispose(){
Dispose_toolhub_Component(instHandle);
}
[DllImport("VEngine_Unity.dll")]
static extern void Dispose_toolhub_Component(IntPtr h);
}

}
