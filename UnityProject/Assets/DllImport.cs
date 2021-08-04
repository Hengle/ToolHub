using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Text;
using System.Runtime.InteropServices;
[ExecuteAlways]
public unsafe class DllImport : MonoBehaviour
{
    [DllImport("VEngine_DLL.dll")]
    static extern void vengine_init_malloc_path(
    byte* path);

    static byte[] GetPath(string pathStr)
    {
        return Encoding.ASCII.GetBytes(pathStr + (char)0);

    }
    void OnEnable()
    {
        string pluginPath = "Assets/Plugins/";
        var mimallocPath = GetPath(pluginPath + "mimalloc.dll");
        fixed (byte* b = mimallocPath)
        {
            vengine_init_malloc_path(b);
        }
        TestClass t = new TestClass();
        Debug.Log(t.Run1(5, 3));
        t.Dispose();
        Debug.Log("Success!");
    }

}
