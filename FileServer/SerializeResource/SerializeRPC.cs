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
            long version)
        {
            CreateSerFileResult result;
            vstd.Guid guid;
            if (ResourceManager.CreateFile(
                db,
                name,
                version,
                out guid))
            {
                RPCSocket.Current.CallRemoteFunction(
                     "SerializeRPC",
                     "CreateFileSuccess",
                     new object[] { name, version, guid.ToString() });
            }
            else
            {
                RPCSocket.Current.CallRemoteFunction(
                     "SerializeRPC",
                     "CreateFileFail",
                     new object[] { name, version, guid.ToString() });
            }

        }
        public static void GetFileID(
            string name,
            long version)
        {
            var guid = ResourceManager.FindFile(db, name, version);
            RPCSocket.Current.CallRemoteFunction(
                "SerializeRPC",
                "FileFindResult",
                new object[] { name, version, guid.ToString() });
        }
        public static void OpenWritableFile(
            vstd.Guid fileID,
            vstd.Guid userID)
        {
            var dict = ResourceManager.openedFiles;
            bool success;
            lock (dict)
            {
                ResourceManager.OpenFile file;
                if (!dict.TryGetValue(fileID, out file))
                {
                    file = new ResourceManager.OpenFile();
                    file.resource = new SerializeResource(fileID.ToString(), true, fileID);
                    dict.Add(fileID, file);
                }
                if (file.writingUser.IsCreated
                     && file.writingUser != userID)
                {
                    success = false;
                }
                else
                {
                    success = true;
                    file.writingUser = userID;
                }
                if (success)
                {
                    RPCSocket.Current.CallRemoteFunction(
                        "SerializeRPC",
                        "RWFileOpenSuccess",
                        fileID.ToString());
                }
                else
                {
                    RPCSocket.Current.CallRemoteFunction(
                        "SerializeRPC",
                        "RWFileOpenedByOthers",
                        fileID.ToString());
                }
            }
        }
        public static void OpenReadableFile(
            vstd.Guid fileID)
        {
            var dict = ResourceManager.openedFiles;

            bool success = true;
            lock (dict)
            {
                ResourceManager.OpenFile file;
                if (!dict.TryGetValue(fileID, out file))
                {
                    if (ResourceManager.FindFile(db, fileID))
                    {
                        file = new ResourceManager.OpenFile();
                        file.resource = new SerializeResource(fileID.ToString(), false, fileID);
                        dict.Add(fileID, file);
                    }
                    else
                    {
                        success = false;
                        return;
                    }
                }
            }
            if (success)
            {
                RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "FileOpenSuccess",
                    fileID.ToString());
            }
            else
            {
                RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "TryOpenNonExistFile",
                    fileID.ToString());
            }
        }
        public static void DeleteFile(
            vstd.Guid fileID,
            vstd.Guid userID)
        {
            uint state;
            var dict = ResourceManager.openedFiles;
            lock (dict)
            {
                ResourceManager.OpenFile file;
                if (!dict.TryGetValue(fileID, out file))
                {
                    state = 0;//Non Exists
                }
                else
                {
                    if (file.writingUser != fileID)
                    {
                        state = 1; // No right
                    }
                    else
                        state = 2;
                }
            }
            switch (state)
            {
                case 0:
                    RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "TryDeleteNonExistFile",
                    fileID.ToString());
                    break;
                case 1:
                    RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "TryDeleteNoRightFile",
                    fileID.ToString());
                    break;
                case 2:
                    RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "DeleteSuccess",
                    fileID.ToString());
                    break;
            }
        }
    }