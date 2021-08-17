using Network;
namespace FileServer
{
    [RPC(RPCLayer.Layer0)]
    public static class ServerRPC_File
    {
        public static MongoDatabase db = null;
        public static RPCSocket rpc = null;
        public static void UploadRequest(UploadCmd uploadCmd)
        {
            if (FileSystem.UpdateFileDataToDB(db.fileCollect, uploadCmd.filePath, uploadCmd.fileData)){
                rpc.CallRemoteFunction(
                    "ClientRPC_File",
                    "UploadResult",
                    new UploadResult {
                        isSuccess = true,
                        uploadState = FileUploadState.Upload,
                        message = "File " + uploadCmd.filePath + " update success!"
                    });
            }
            else
            {
                rpc.CallRemoteFunction(
                    "ClientRPC_File",
                    "UploadResult",
                    new UploadResult
                    {
                        isSuccess = true,
                        uploadState = FileUploadState.Added,
                        message = "File " + uploadCmd.filePath + " added success!"
                    });
            }
        }
        //public static void DownloadRequest()
    }
}