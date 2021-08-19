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
    [Network.RPC(Network.RPCLayer.All)]
    public static class RPCTest
    {
        public static void PrintString(object str)
        {
            string s = (string)str;
            Console.WriteLine("From Remote: " + s);
        }
    }
    unsafe class Program
    {
        const int PORT = 2002;
        static void Main(string[] args)
        {
            RPCReflector.LoadRPCFunctor(System.Reflection.Assembly.GetExecutingAssembly(), RPCLayer.All);
            Console.WriteLine("Start server");
            TcpListener listener = new TcpListener(System.Net.IPAddress.Any, PORT);
            listener.Start();
            while (true) {
                RPCSocket rpcSocket = new RPCSocket(listener, PORT);
                Console.WriteLine("Connected to client!");
            }
        }
    }
}
