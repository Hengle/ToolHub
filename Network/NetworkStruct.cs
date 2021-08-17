namespace Network
{
    [System.Serializable]
    public struct UploadCmd
    {
        public string filePath;
        public byte[] fileData;
    }
    [System.Serializable]
    public struct DownloadCmd
    {
        //public string targetFilePath;
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