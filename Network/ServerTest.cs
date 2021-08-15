using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Http;
using System.Net.Sockets;
using System.IO;
using Native;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace client
{

    class Program
    {
        static string ipAddress;
        unsafe class ServerProcessor : Network.IStreamerVisitor, IDisposable
        {
            TcpListener server = null;
            NetworkStream stream;
            TcpClient client;
            byte[] bytes = new byte[1024];
            public ServerProcessor()
            {
                IPAddress localAddr = IPAddress.Parse(ipAddress);

                // TcpListener server = new TcpListener(port);
                server = new TcpListener(localAddr, port);

                // Start listening for client requests.
                server.Start();
                client = server.AcceptTcpClient();
                stream = client.GetStream();
            }

            public void GetNextByteArray(out byte[] byteArr, out ulong usedByteLen)
            {
                usedByteLen = (ulong)(stream.Read(bytes, 0, bytes.Length));
                byteArr = bytes;
            }
            public Task Execute(IntPtr data, ulong byteLength)
            {
                return Task.Run(() =>
                {
                    ulong ofst = 0;
                    Console.WriteLine(Memory.DeSerString((byte*)data.ToPointer(), ref ofst));
                });
            }
            public void Dispose()
            {
                server.Stop();
            }
        }
        const int port = 13000;

      
        unsafe static void Client()
        {

            TcpClient client = new TcpClient(ipAddress, port);
            using (NetworkStream stream = client.GetStream())
            {

                //byte[] bytes = new byte[5];

                while (true)
                {
                    string s = Console.ReadLine();
                    byte[] newBytes = null;
                    ulong ofst = s.Serialize(ref newBytes);
                    stream.Write(newBytes, 0, (int)ofst);
                }
            }
            client.Close();

        }


        public static void Main()
        {
            Memory.vengine_init_malloc();
            Console.WriteLine("Server (Y) or Client(N)");


            ipAddress = Network.NetworkUtility.GetLocalIP();
            string s = Console.ReadLine();
            if (s == "Y" || s == "y")
            {
                Console.WriteLine("Start HTTP Server!");
                Network.DataStreamer streamer = new Network.DataStreamer();
                using (var v = new ServerProcessor())
                {
                    while (true)
                    {
                        streamer.StreamNext_InterSize(v);
                    }
                }
            }
            else
            {
                Console.WriteLine("Start HTTP Client!");
                Client();
            }
            Console.Read();
        }
    }
}