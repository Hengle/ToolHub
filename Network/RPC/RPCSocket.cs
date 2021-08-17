using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Collections.Concurrent;
using System.Net.Sockets;
using System.Runtime.Serialization.Formatters.Binary;
using System.Threading;

namespace Network
{
    public class RPCSocket : IDisposable
    {
        TcpListener listener = null;
        NetworkStream stream;
        TcpClient client;
        Task readTask;
        Task writeTask;
        BinaryFormatter formatter;
        AutoResetEvent writeThreadLocker;
        bool classEnabled = true;
        public struct CallCmd
        {
            public string className;
            public string funcName;
            public object argument;
            public CallCmd(
            string className,
            string funcName,
            object argument = null)
            {
                this.className = className;
                this.funcName = funcName;
                this.argument = argument;
            }
        }
        ConcurrentQueue<CallCmd> writeCmd = new ConcurrentQueue<CallCmd>();
        private void StartThread()
        {
            writeThreadLocker = new AutoResetEvent(true);
            formatter = new BinaryFormatter();
            readTask = Task.Run(() =>
            {
                try
                {
                    while (classEnabled)
                    {
                        RPCReflector.ExecuteStream(
                            stream,
                            formatter);
                    }
                }
                catch(Exception e)
                {
                    Console.WriteLine(e.Message);
                }
                finally
                {

                }
            });
            writeTask = Task.Run(() =>
            {
                while (classEnabled)
                {
                    writeThreadLocker.WaitOne();
                    CallCmd cmd;
                    while (writeCmd.TryDequeue(out cmd))
                    {
                        RPCReflector.CallFunc(
                            stream,
                            formatter,
                            cmd.className,
                            cmd.funcName,
                            cmd.argument);
                    }
                }
            });
        }
        public RPCSocket(int port)
        {
            listener = new TcpListener(IPAddress.Any, port);
            listener.Start();
            client = listener.AcceptTcpClient();
            stream = client.GetStream();
            StartThread();
        }

        public RPCSocket(string connectTarget, int port)
        {
            client = new TcpClient(connectTarget, port);
            stream = client.GetStream();
            StartThread();
        }
        public void CallRemoteFunction(string className, string funcName, object argument = null)
        {
            writeCmd.Enqueue(new CallCmd(className, funcName, argument));
            writeThreadLocker.Set();
        }
        public void CallRemoteFunctions(IEnumerable<CallCmd> cmds)
        {
            foreach(var i in cmds)
            {
                writeCmd.Enqueue(i);
            }
            writeThreadLocker.Set();
        }
        public void Dispose()
        {
            classEnabled = false;
            stream.Dispose();
            client.Dispose();
            if (listener != null)
            {
                listener.Stop();
                listener = null;
            }

            writeThreadLocker.Set();
            readTask.Wait();
            writeTask.Wait();
            writeThreadLocker.Dispose();
        }
    }
}
