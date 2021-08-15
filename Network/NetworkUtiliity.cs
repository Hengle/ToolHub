using System;
using System.Net;
using System.Net.Sockets;

namespace Network
{
    public static class NetworkUtility
    {
        public static string GetLocalIP(AddressFamily addressFamily = AddressFamily.InterNetwork)
        {
            try
            {
                string HostName = Dns.GetHostName();
                IPHostEntry IpEntry = Dns.GetHostEntry(HostName);
                for (int i = 0; i < IpEntry.AddressList.Length; i++)
                {
                    if (IpEntry.AddressList[i].AddressFamily == addressFamily)
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

    }
}