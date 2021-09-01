using System.Collections.Generic;

namespace Network
{
    [VSerializable]
    public struct Pair
    {
        public object first;
        public object second;
    }
    [VSerializable]
    public struct UploadCmd
    {
        public vstd.Guid guid;
        public string filePath;
        public byte[] fileData;
        public byte[] metaData;
    }
    [VSerializable]
    public struct DownloadCmd
    {
        public vstd.Guid tarGuid;
    }
    [VSerializable]
    public struct DownloadResult
    {
        public bool success;
        public string message;
        public string filePath;
        public byte[] fileResult;
        public byte[] metaFileResult;
    }
    [VSerializable]
    public enum FileUploadState : byte
    {
        Upload,
        Added
    }
    [VSerializable]
    public struct UploadResult
    {
        public bool isSuccess;
        public FileUploadState uploadState;
        public string message;
    }
    [VSerializable]
    public enum SerializeValueType : byte
    {
        None,
        Structure,
        String,
        Bool,
        Int,
        Float,
        Reference,
        Num
    };
    [VSerializable]
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
        public byte type;//SerializeValueType
        public bool isArray;
    }

    [VSerializable]
    public struct SerializeStruct
    {
        //string, SerializeMember
        public List<Pair> members;
    }


    [VSerializable]
    public struct CreateSerFileResult
    {
        public bool isSuccess;
        public string fileName;
    }
}