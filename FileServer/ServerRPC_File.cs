using Network;
using System.Threading;
using System;
namespace FileServer
{
    [RPC(RPCLayer.Layer0)]
    public static class ServerRPC_File
    {
        public static MongoDatabase db = null;
        public static void UploadRequest(UploadCmd uploadCmd)
        {
            UploadResult result;
            if (FileSystem.UpdateFileDataToDB(
                 db.fileCollect,
                 uploadCmd.guid,
                 uploadCmd.filePath,
                 uploadCmd.fileData,
                 uploadCmd.metaData))
            {
                result = new UploadResult
                {
                    isSuccess = true,
                    uploadState = FileUploadState.Upload,
                    message = "File " + uploadCmd.filePath + " update success!"
                };
            }
            else
            {
                result = new UploadResult
                {
                    isSuccess = true,
                    uploadState = FileUploadState.Added,
                    message = "File " + uploadCmd.filePath + " added success!"
                };
            }
            var rpc = RPCSocket.ThreadLocalRPC;
            rpc.CallRemoteFunction(
                  "ClientRPC_File",
                  "UploadCallback",
                  result);
        }
        public static void DownloadRequest(DownloadCmd cmd)
        {
            DownloadResult callback = new DownloadResult();
            int result = FileSystem.TryDownload(db.fileCollect, cmd.tarGuid, out callback.filePath, out callback.fileResult, out callback.metaFileResult);
            switch (result)
            {
                case 0:
                    {
                        callback.success = true;
                        callback.message = "Successfully download file: " + callback.filePath;

                    }
                    break;
                case 1:
                    {
                        callback.success = false;
                        callback.message = "Download file failed, cannot find guid: " + cmd.tarGuid.ToString();
                        callback.filePath = null;
                        callback.fileResult = null;
                        callback.metaFileResult = null;

                    }
                    break;
                case 2:
                    {
                        callback.fileResult = null;
                        callback.metaFileResult = null;
                        callback.success = false;
                        callback.message = "Download file failed, cannot file path: " + callback.filePath;

                    }
                    break;
            }
            var rpc = RPCSocket.ThreadLocalRPC;
            rpc.CallRemoteFunction(
                "ClientRPC_File",
                "DownloadCallback",
                callback);
        }

        public static void TestRPC(string value)
        {
            Console.WriteLine("Get: " + value);
            var rpc = RPCSocket.ThreadLocalRPC;
            rpc.CallRemoteFunction(
                "ClientRPC_File",
                "TestRPC",
                "fuck " + value);
        }
        //public static void DownloadRequest()
    }
}