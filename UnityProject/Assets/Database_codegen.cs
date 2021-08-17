using System.Runtime.InteropServices;
using System;
using System.Collections.Generic;
using System.Collections;
using MPipeline;
namespace toolhub{
unsafe class Database: IDisposable{
public IntPtr GetResourceDatabase(){
IntPtr v=default;
toolhub_Database_GetResourceDatabase(ref v,instHandle);
return v;
}
public IntPtr GetSceneDatabase(int sceneIndex){
IntPtr v=default;
toolhub_Database_GetSceneDatabase(ref v,instHandle,sceneIndex);
return v;
}
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Database_GetResourceDatabase(ref IntPtr F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8);
[DllImport("VEngine_Unity.dll")]
static extern void toolhub_Database_GetSceneDatabase(ref IntPtr F32D2BF6260A4FBD,IntPtr AFD920F282E74FF8,int sceneIndex);
public IntPtr instHandle{get; private set;}
public Database(IntPtr newHandle){
instHandle = newHandle;
}
public Database(){
instHandle=Create_toolhub_Database0();
}
[DllImport("VEngine_Unity.dll")]
static extern IntPtr Create_toolhub_Database0();
public void Dispose(){
Dispose_toolhub_Database(instHandle);
}
[DllImport("VEngine_Unity.dll")]
static extern void Dispose_toolhub_Database(IntPtr h);
}

}
