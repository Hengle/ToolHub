using System.IO;
namespace Network
{
    public class FileUtility
    {
        public static void WriteFile(string path, byte[] bytes)
        {
            string dir = Path.GetDirectoryName(path);
            if (!string.IsNullOrEmpty(dir))
                Directory.CreateDirectory(dir);
            File.WriteAllBytes(path, bytes);
        }
    }
}