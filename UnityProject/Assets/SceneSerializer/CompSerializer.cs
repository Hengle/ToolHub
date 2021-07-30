using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Reflection;
public class Test
{
    private int test0;
    [System.NonSerialized]
    public int test3;
    public int test1;
    [SerializeField]
    private int test2;
}
[ExecuteAlways]
public class CompSerializer : MonoBehaviour
{
    public static void GetSerializedVariables(object comp, List<FieldInfo> infos)
    {
        var lst = comp.GetType().GetFields(BindingFlags.Public | BindingFlags.Instance);
        foreach (var i in lst)
        {
            if (i.GetCustomAttribute<System.NonSerializedAttribute>() == null)
            {
                infos.Add(i);
            }
        }
        lst = comp.GetType().GetFields(BindingFlags.NonPublic | BindingFlags.Instance);
        foreach (var i in lst)
        {
            if (i.GetCustomAttribute<SerializeField>() != null)
            {
                infos.Add(i);
            }
        }
    }
    public static void GetSerializedProperty(object comp, List<PropertyInfo> props)
    {
        var lst = comp.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance);
        foreach (var i in lst)
        {
            if (i.CanRead && i.CanWrite)
            {
                props.Add(i);
            }
        }
    }

    public Component comp;
    void Run()
    {
        List<FieldInfo> infos = new List<FieldInfo>();
        GetSerializedVariables(comp, infos);
        foreach (var i in infos)
        {
            Debug.Log(i.Name);
        }
        List<PropertyInfo> props = new List<PropertyInfo>();
        GetSerializedProperty(comp, props);
        foreach(var i in props)
        {
            Debug.Log(i.Name);
        }
    }

    void OnEnable()
    {
        Run();
    }
    void OnDisable()
    {
        Run();
    }

}
