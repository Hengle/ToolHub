using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Text;
using System.Runtime.InteropServices;
using MPipeline;
using System;
unsafe struct CSharpString
{
    char* ptr;
    ulong size;
    public CSharpString(string s)
    {
        ptr = s.Ptr();
        size = (ulong)s.Length;
    }
}
struct BinaryArray
{
    public IntPtr ptr;
    public int count;
    public int stride;
}

enum ComponentType
{
    Assets,
    Component,
    Value
}

[ExecuteAlways]
public unsafe class DllImport : MonoBehaviour
{
    [DllImport("VEngine_Unity.dll")]
    static extern void DllImport_Init(
    byte* path);

    static byte[] GetPath(string pathStr)
    {
        return Encoding.ASCII.GetBytes(pathStr + (char)0);

    }
    void OnEnable()
    {
        string pluginPath = "Assets/Plugins/";
        var mimallocPath = GetPath(pluginPath);
        fixed (byte* b = mimallocPath)
        {
            DllImport_Init(b);
        }
    }

}
