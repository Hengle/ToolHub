using System;
using System.Collections.Generic;
using Native;
using MongoDB.Driver;
using MongoDB.Bson;
using System.IO;
using Network;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Net.Sockets;

namespace FileServer
{

    [Network.RPC(RPCLayer.All)]
    public static class Print_Server
    {
        public static void Print(string str, string str1)
        {
            Console.WriteLine("From Client: " + str);
            RPCSocket.Current.CallRemoteFunction(
                "Print_Client",
                "Print",
                new object[] {
                "calling me " + str + "? and",
                "calling me " + str1 + "?"
                });
        }
    }

    unsafe class Program
    {
        const int PORT = 2002;
        static void Server()
        {
            ServerRPC_File.db = new MongoDatabase();

            RPCReflector.LoadRPCFunctor(System.Reflection.Assembly.GetExecutingAssembly(), RPCLayer.All);
            Console.WriteLine("Start server");
            TcpListener listener = new TcpListener(System.Net.IPAddress.Any, PORT);
            listener.Start();
            while (true)
            {
                RPCSocket rpcSocket = new RPCSocket(listener, PORT);
                Console.WriteLine("Connected to client!");
            }
        }
        static void TestSerDe()
        {
            vstd.Guid guid = new vstd.Guid("1BDE6DE21BC243369A887D86EAB3BC98");
            Console.WriteLine("read(y) or write(any key)");
            string s = Console.ReadLine();
            if (s == "y" || s == "Y")
            {
                using (SerializeResource res = new SerializeResource("fuckTest.bytes", false, guid))
                {
                    res.serObj.Print();
                }
            }
            else
            {
                using (SerializeResource res = new SerializeResource("fuckTest.bytes", true, guid))
                {
                    SerializeStruct strct = new SerializeStruct { members = new Dictionary<string, SerializeMember>() };
                    strct.members.Add("string_key", new SerializeMember
                    {
                        isArray = true,
                        type = SerializeValueType.String,
                        value = new List<string> {
                            "string_value0" ,
                            "string_value1" ,
                            "string_value2" ,
                            "string_value3"
                        }
                    });
                    res.SetStruct(new vstd.Guid(true), strct);
                }
            }
        }
        static void Main(string[] args)
        {
            Server();
        }
    }
}
