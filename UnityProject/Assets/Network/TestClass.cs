using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Network;
namespace Network
{
    [VSerializable]
    public struct SerializeMember
    {
        /**
         * Value possible type:
         * vstd.Guid List<vstd.Guid>
         * string List<string>
         * bool List<bool>
         * long List<long>
         * double List<double>
         * vstd.Guid List<vstd.Guid>
         * */
        public object value;
        public byte type;//SerializeValueType
        public bool isArray;
    }

    [VSerializable]
    public struct SerializeStruct
    {
        //string, SerializeMember
        public List<Pair> members;
    }
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
                new object[] { new vstd.Guid(fileID), new vstd.Guid(userID) });
        }
        if (Input.GetKeyDown(KeyCode.Alpha4))
        {
            socket.CallRemoteFunction(
                "SerializeRPC",
                "OpenWritableFile",
                new object[] { new vstd.Guid(fileID), new vstd.Guid(userID) });
        }
        if (Input.GetKeyDown(KeyCode.Alpha5))
        {
            socket.CallRemoteFunction(
                "SerializeRPC",
                "GetFileStructure",
                new object[] { new vstd.Guid(fileID) });
        }
        if (Input.GetKeyDown(KeyCode.Space))
        {
            socket.CallRemoteFunction("SerializeRPC", "Disconnect");
        }
    }
}
