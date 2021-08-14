using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Http;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace client
{
    struct DLLInitializer
    {
        [DllImport("VEngine_DLL.dll")]
        static extern void vengine_init_malloc();
        public DLLInitializer(bool initialize)
        {
            if (!initialize) return;
            vengine_init_malloc();
        }
    }
    class Program
    {
        static string ipAddress;
        static DLLInitializer dllInitializer = new DLLInitializer(true);
        const int port = 13000;

        public static string GetLocalIP()
        {
            try
            {
                string HostName = Dns.GetHostName();
                IPHostEntry IpEntry = Dns.GetHostEntry(HostName);
                for (int i = 0; i < IpEntry.AddressList.Length; i++)
                {
                    if (IpEntry.AddressList[i].AddressFamily == AddressFamily.InterNetwork)
                    {
                        return IpEntry.AddressList[i].ToString();
                    }
                }
                return "";
            }
            catch
            {
                return "";
            }
        }

        static void Server()
        {
            TcpListener server = null;
            try
            {
                IPAddress localAddr = IPAddress.Parse(ipAddress);

                // TcpListener server = new TcpListener(port);
                server = new TcpListener(localAddr, port);

                // Start listening for client requests.
                server.Start();

                // Buffer for reading data
                byte[] bytes = new byte[5];
                string data;
                while (true)
                {
                    Console.Write("Waiting for a connection... ");

                    // Perform a blocking call to accept requests.
                    // You could also use server.AcceptSocket() here.
                    TcpClient client = server.AcceptTcpClient();

                    Console.WriteLine("Connected!");

                    data = null;

                    // Get a stream object for reading and writing
                    NetworkStream stream = client.GetStream();

                    int i;

                    // Loop to receive all the data sent by the client.
                    while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        // Translate data bytes to a ASCII string.
                        data = System.Text.Encoding.ASCII.GetString(bytes, 0, i);
                        Console.WriteLine("Received: {0}", data);

                        // Process the data sent by the client.
                        data = data.ToUpper();

                        byte[] msg = System.Text.Encoding.ASCII.GetBytes(data);

                        // Send back a response.
                        stream.Write(msg, 0, msg.Length);
                        Console.WriteLine("Sent: {0}", data);
                    }

                    // Shutdown and end connection
                    client.Close();
                }
            }
            catch
            {
                if (server != null) server.Stop();
            }
        }
        static void Client()
        {

            TcpClient client = new TcpClient(ipAddress, port);
            using (NetworkStream stream = client.GetStream())
            {

                //byte[] bytes = new byte[5];
                while (true)
                {
                    string s = Console.ReadLine();
                    stream.Write(Encoding.ASCII.GetBytes(s));
                }
            }
            client.Close();

        }


        public static void Main()
        {
           
            ipAddress = GetLocalIP();
            Console.WriteLine("Server (Y) or Client(N)");
            string s = Console.ReadLine();
            if (s == "Y" || s == "y")
            {
                Console.WriteLine("Start HTTP Server!");
                Server();
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