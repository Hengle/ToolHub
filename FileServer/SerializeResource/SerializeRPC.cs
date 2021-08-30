using System;
using Network;

namespace FileServer
{

    [RPC(RPCLayer.All)]
    public static class SerializeRPC
    {
        public static MongoDatabase db;
        public static void CreateFile(
            string name,
            vstd.Guid userID,
            vstd.Guid fileID)
        {
            CreateSerFileResult result;
            if (ResourceManager.CreateFile(
                db,
                name,
                fileID, userID))
            {
                result = new CreateSerFileResult
                {
                    fileName = name,
                    isSuccess = true
                };
            }
            else
            {
                result = new CreateSerFileResult
                {
                    fileName = name,
                    isSuccess = true
                };
            }
            RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "CreateFileFeedBack",
                    result);
        }

        public static void OpenWritableFile(
            vstd.Guid fileID,
            vstd.Guid userID)
        {

        }
    }
}