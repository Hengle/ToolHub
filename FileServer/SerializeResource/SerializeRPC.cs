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
            string fileIDStr,
            string userIDStr)
        {
            try
            {
                vstd.Guid fileID = new vstd.Guid(fileIDStr);
                vstd.Guid userID = new vstd.Guid(userIDStr);
                var dict = ResourceManager.openedFiles;
                uint state = 0;
                lock (dict)
                {
                    ResourceManager.OpenFile file;
                    if (!dict.TryGetValue(fileID, out file))
                    {
                        if (ResourceManager.FindFile(db, fileID))
                        {
                            file = new ResourceManager.OpenFile();
                            file.resource = new SerializeResource(fileID.ToString(), true, fileID);
                            dict.Add(fileID, file);
                        }
                        else
                        {
                            state = 1; //non-exist
                            file = null;
                        }
                    }
                    if (file != null)
                    {
                        if (file.writingUser.IsCreated
                             && file.writingUser != userID)
                        {
                            state = 2; //opened;
                        }
                        else
                        {
                            file.writingUser = userID;
                        }
                    }
                }
                switch (state)
                {
                    case 0:
                        RPCSocket.Current.CallRemoteFunction(
                        "SerializeRPC",
                        "RWFileOpenSuccess",
                        fileID.ToString());
                        break;
                    case 1:
                        RPCSocket.Current.CallRemoteFunction(
                      "SerializeRPC",
                      "RWFileNonExist",
                      fileID.ToString());
                        break;
                    case 2:
                        RPCSocket.Current.CallRemoteFunction(
                        "SerializeRPC",
                        "RWFileOpenedByOthers",
                        fileID.ToString());
                        break;
                }
            }
            catch (ArgumentException ex)
            {
                Console.WriteLine(ex.Message.ToString());

                RPCSocket.Current.CallRemoteFunction(
                "SerializeRPC",
                "WrongGuidFormat");
            }
        }
        public static void OpenReadableFile(
            string fileIDStr)
        {
            try
            {
                vstd.Guid fileID = new vstd.Guid(fileIDStr);
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
            catch (ArgumentException ex)
            {
                RPCSocket.Current.CallRemoteFunction(
                "SerializeRPC",
                "WrongGuidFormat");
            }
        }

        public static void Disconnect()
        {
            RPCSocket.DisposeCurrent();
        }
        public static void DeleteFile(
            string fileIDStr,
            string userIDStr)
        {
            try
            {
                vstd.Guid fileID = new vstd.Guid(fileIDStr);
                vstd.Guid userID = new vstd.Guid(userIDStr);
                uint state;
                var dict = ResourceManager.openedFiles;
                lock (dict)
                {
                    ResourceManager.OpenFile file;
                    if (!dict.TryGetValue(fileID, out file))
                    {
                        if (ResourceManager.DeleteFile(db, fileID))
                            state = 2;
                        else
                            state = 0;//Non Exists
                    }
                    else
                    {
                        if (file.writingUser != userID)
                        {
                            state = 1; // No right
                        }
                        else
                        {
                            ResourceManager.DeleteFileNoCheck(db, fileID);
                            file.resource.Dispose();
                            try
                            {
                                System.IO.File.Delete(file.resource.path);
                            }
                            catch { }
                            dict.Remove(fileID);
                            state = 2;
                        }
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
            catch (ArgumentException ex)
            {
                RPCSocket.Current.CallRemoteFunction(
                "SerializeRPC",
                "WrongGuidFormat");
            }
        }

    }
}