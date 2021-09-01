using System;
using Network;

namespace FileServer
{
    public static class SerUtil
    {
        public static ResourceManager.OpenFile GetReadonlyFile(in vstd.Guid fileID)
        {
            ResourceManager.OpenFile res;
            if (!ResourceManager.openedFiles.TryGetValue(fileID, out res)) return null;
            return res;
        }
        public static ResourceManager.OpenFile GetWritableFile(
            in vstd.Guid fileID,
            in vstd.Guid userID)
        {
            var res = GetReadonlyFile(fileID);
            if (res == null) return null;
            if (res.writingUser != userID) return null;
            return res;
        }

    }
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
                     new object[] { name, version, guid });
            }
            else
            {
                RPCSocket.Current.CallRemoteFunction(
                     "SerializeRPC",
                     "CreateFileFail",
                     new object[] { name, version, guid });
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
                new object[] { name, version, guid });
        }
        public static void OpenWritableFile(
            vstd.Guid fileID,
            vstd.Guid userID)
        {
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
                    fileID);
                    break;
                case 1:
                    RPCSocket.Current.CallRemoteFunction(
                  "SerializeRPC",
                  "RWFileNonExist",
                  fileID);
                    break;
                case 2:
                    RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "RWFileOpenedByOthers",
                    fileID);
                    break;
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
                    fileID);
            }
            else
            {
                RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "TryOpenNonExistFile",
                    fileID);
            }

        }

        public static void Disconnect()
        {
            RPCSocket.DisposeCurrent();
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
                    fileID);
                    break;
                case 1:
                    RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "TryDeleteNoRightFile",
                    fileID);
                    break;
                case 2:
                    RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "DeleteSuccess",
                    fileID);
                    break;
            }
        }
        public static void GetFileStructure(
            vstd.Guid fileID)
        {
            var file = SerUtil.GetReadonlyFile(fileID);
            if (file == null)
            {
                RPCSocket.Current.CallRemoteFunction(
                  "SerializeRPC",
                  "FileNotOpened",
                  fileID);
            }
            else
            {
                RPCSocket.Current.CallRemoteFunction(
                    "SerializeRPC",
                    "FileReadSuccess",
                    file.resource.serObj.SerializeToByteArray());
            }
        }
    }
}