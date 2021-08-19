using Network;
using System;
namespace FileClient
{
    class Program
    {
        static void Main(string[] args)
        {
            RPCReflector.LoadRPCFunctor(System.Reflection.Assembly.GetExecutingAssembly(), RPCLayer.All);
            Console.WriteLine("Start client");
            RPCSocket rpc = new RPCSocket("127.0.0.1", 2002);
            Console.WriteLine("Connected to server!");
            while (true)
            {
                string s = Console.ReadLine();
                if (s == "exit") {
                    rpc.Dispose();
                }
                else
                {
                    rpc.CallRemoteFunction(
                    "ServerRPC_File",
                    "TestRPC",
                    s);
                }
            }
            Console.WriteLine("finished");
        }
    }
}