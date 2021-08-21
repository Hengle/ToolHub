using Network;
using System;
using System.IO;
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
            Console.WriteLine("Input 0: upload file");
            Console.WriteLine("Input 1: download file");
            while (true)
            {
                string s = Console.ReadLine();
                if (s == "0")
                {
                    Console.WriteLine("Input file path: ");
                    string filePath = Console.ReadLine();
                    rpc.CallRemoteFunction(
                        "ServerRPC_File",
                        "UploadRequest",
                        new UploadCmd
                        {
                            fileData = File.ReadAllBytes(filePath),
                            metaData = File.ReadAllBytes(filePath + ".meta"),
                            guid = vstd.Guid.GetGuidFromUnityMeta(filePath + ".meta"),
                            filePath = filePath
                        });
                }
            }
            Console.WriteLine("finished");
        }
    }
}