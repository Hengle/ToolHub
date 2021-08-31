using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Network;
[Network.RPC(RPCLayer.Layer0)]
public static class SerializeRPC
{
    public static void CreateFileSuccess(
        string name,
        long version,
        string guid)
    {
        Debug.Log("Create " + name + " sccuess!");
    }
    public static void CreateFileFail(
        string name,
        long version,
        string guid)
    {
        Debug.Log("Create " + name + " fail!");

    }
    public static void FileFindResult(
        string name,
        long version,
        string guid)
    {
        Debug.Log("" + name + "'s guid is: " + guid);
        TestClass.current.fileID = guid;

    }
    public static void RWFileOpenSuccess(string guid)
    {
        Debug.Log("Open writable file " + guid + " success!");
    }
    public static void RWFileOpenedByOthers(string guid) { 
        Debug.Log("file " + guid + " already opened!");
    }
    public static void RWFileNonExist(string guid) { 
        Debug.Log("file " + guid + " non-exist!");

    }
    public static void FileOpenSuccess(string guid) {
        Debug.Log("Open read-only file " + guid + " success!");

    }
    public static void TryOpenNonExistFile(string guid) {
        Debug.Log("file " + guid + " non-exist!");
    }
    public static void TryDeleteNonExistFile(string guid)
    {
        Debug.Log("Try delete non-exists file: " + guid);
    }
    public static void TryDeleteNoRightFile(string guid)
    {
        Debug.Log("Can not delete other's file: " + guid);
    }
    public static void DeleteSuccess(string guid)
    {
        Debug.Log("Delete file " + guid + " success!");
    }
    public static void WrongGuidFormat()
    {
        Debug.Log("Sended wrong format guid!");
    }
}
