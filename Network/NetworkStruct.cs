using System.Collections.Generic;
namespace Network
{
    [System.Serializable]
    public struct StructTest
    {
        public string v;
    }
    [System.Serializable]
    public struct UploadCmd
    {
        public vstd.Guid guid;
        public string filePath;
        public byte[] fileData;
        public byte[] metaData;
    }
    [System.Serializable]
    public struct DownloadCmd
    {
        public vstd.Guid tarGuid;
    }
    [System.Serializable]
    public struct DownloadResult
    {
        public bool success;
        public string message;
        public string filePath;
        public byte[] fileResult;
        public byte[] metaFileResult;
    }
    [System.Serializable]
    public enum FileUploadState : byte
    {
        Upload,
        Added
    }
    [System.Serializable]
    public struct UploadResult
    {
        public bool isSuccess;
        public FileUploadState uploadState;
        public string message;
    }
    [System.Serializable]
    public enum SerializeValueType : byte
    {
        None,
        Structure,
        String,
        Bool,
        Int,
        Float,
        Reference,
        Guid,
        Num
    };
    [System.Serializable]
    public struct SerializeMember
    {
        /**
         * Value possible type:
         * vstd.Guid List<vstd.Guid>
         * string List<string>
         * bool List<bool>
         * long List<long>
         * double List<double>
         * vstd.Guid List<vstd.Guid>
         * */
        public object value;
        public SerializeValueType type;
        public bool isArray;
    }

    [System.Serializable]
    public struct SerializeStruct
    {
        public Dictionary<string, SerializeMember> members;
    }


    [System.Serializable]
    public struct CreateSerFileResult
    {
        public bool isSuccess;
        public string fileName;
    }
}