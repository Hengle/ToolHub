using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Native;
namespace AssetDatabase
{
    public unsafe static class FileSystem
    {
        public static vstd.MD5 GetFileMD5(string filePath)
        {
            var bytes = File.ReadAllBytes(filePath);
            fixed (byte* bb = bytes)
            {
                return new vstd.MD5(bb, (ulong)bytes.LongLength);
            }
        }
    }
}
