using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Network;
namespace Network
{
    [System.Serializable]
    public struct CreateSerFileResult
    {
        public bool isSuccess;
        public string fileName;
    }
}
namespace FileServer
{

}
[Network.RPC(RPCLayer.All)]
public static class Print_Client
{
    public static void Print(string str, string str1)
    {
        Debug.Log("From Server: " + str);
        Debug.Log("From Server: " + str1);
    }
}

public unsafe class TestClass : MonoBehaviour
{
    RPCSocket socket;
    private void Awake()
    {
        RPCReflector.LoadRPCFunctor(System.Reflection.Assembly.GetExecutingAssembly(), RPCLayer.All);
        try
        {
            socket = new RPCSocket("127.0.0.1", 2002);
        }
        catch (System.Exception exp)
        {
            Debug.LogError(exp.Message);
        }
        current = this;
        byte[] bytes = System.IO.File.ReadAllBytes("UserIdentity.txt");
        fixed (byte* b = bytes)
        {
            vstd.MD5 md5 = new vstd.MD5(b, (ulong)bytes.LongLength);
            userID = md5.ToGUID().ToString();
        }
    }
    private void OnDestroy()
    {
        socket.Dispose();
    }

    public string fileName = "test_file";
    public string fileID;
    public string userID = "";
    public static TestClass current = null;
    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.Alpha1))
        {
            socket.CallRemoteFunction(
                "SerializeRPC",
                "CreateFile",
                new object[] { fileName, (long)0 });
        }
        if (Input.GetKeyDown(KeyCode.Alpha2))
        {
            socket.CallRemoteFunction(
                "SerializeRPC",
                "GetFileID",
                new object[] { fileName, (long)0 });
        }
        if (Input.GetKeyDown(KeyCode.Alpha3))
        {
            socket.CallRemoteFunction(
                "SerializeRPC",
                "DeleteFile",
                new object[] { fileID, userID });
        }
        if (Input.GetKeyDown(KeyCode.Alpha4))
        {
            socket.CallRemoteFunction(
                "SerializeRPC",
                "OpenWritableFile",
                new object[] { fileID, userID });
        }
        if (Input.GetKeyDown(KeyCode.Space))
        {
            socket.CallRemoteFunction("SerializeRPC", "Disconnect");
        }
    }
}
