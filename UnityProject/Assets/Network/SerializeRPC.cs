using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Network;
using vstd;
[VSerializable]
public struct Pair
{
    public object first;
    public object second;
}

[Network.RPC(RPCLayer.Layer0)]
public static class SerializeRPC
{
    public static void CreateFileSuccess(
        string name,
        long version,
        vstd.Guid guid)
    {
        Debug.Log("Create " + name + " sccuess!");
    }
    public static void CreateFileFail(
        string name,
        long version,
        vstd.Guid guid)
    {
        Debug.Log("Create " + name + " fail!");

    }
    public static void FileFindResult(
        string name,
        long version,
        vstd.Guid guid)
    {
        Debug.Log("" + name + "'s guid is: " + guid);
        TestClass.current.fileID = guid.ToString();

    }
    public static void RWFileOpenSuccess(vstd.Guid guid)
    {
        Debug.Log("Open writable file " + guid + " success!");
    }
    public static void RWFileOpenedByOthers(vstd.Guid guid)
    {
        Debug.Log("file " + guid + " already opened!");
    }
    public static void RWFileNonExist(vstd.Guid guid)
    {
        Debug.Log("file " + guid + " non-exist!");

    }
    public static void FileOpenSuccess(vstd.Guid guid)
    {
        Debug.Log("Open read-only file " + guid + " success!");

    }
    public static void TryOpenNonExistFile(vstd.Guid guid)
    {
        Debug.Log("file " + guid + " non-exist!");
    }
    public static void TryDeleteNonExistFile(vstd.Guid guid)
    {
        Debug.Log("Try delete non-exists file: " + guid);
    }
    public static void TryDeleteNoRightFile(vstd.Guid guid)
    {
        Debug.Log("Can not delete other's file: " + guid);
    }
    public static void DeleteSuccess(vstd.Guid guid)
    {
        Debug.Log("Delete file " + guid + " success!");
    }

    public static void FileNotOpened(vstd.Guid guid)
    {
        Debug.Log("Can not find file " + guid);

    }
    public static void FileReadSuccess(byte[] bytes)
    {
        using (SerializeObject obj = new SerializeObject(true))
        {
            obj.Read(bytes);
            obj.Print();
        }
    }
}
