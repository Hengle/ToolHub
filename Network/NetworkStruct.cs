namespace Network
{
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
}