using Network;
using System.IO;
using System;
namespace FileClient
{
    [RPC(RPCLayer.Layer1)]
    public static class ClientRPC_File
    {
        public static void UploadCallback(UploadResult result)
        {
            Console.WriteLine(result.message);
        }
        public static void DownloadCallback(DownloadResult result)
        {
            Console.WriteLine(result.message);
            if (!result.success) return;
            FileUtility.WriteFile(result.filePath, result.fileResult);
            FileUtility.WriteFile(result.filePath + ".meta", result.metaFileResult);
            //Console.WriteLine()
        }
    }
}