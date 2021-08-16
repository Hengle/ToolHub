using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Native;
using MongoDB.Driver;
using MongoDB.Bson;

namespace AssetDatabase
{
    public struct FileData
    {
        public vstd.MD5 fileMD5;
        public vstd.Guid fileGuid;
        public long fileSize;
        public string filePath;
    }
    public unsafe static class FileSystem
    {
        public static vstd.MD5 CalcFileMD5(string filePath, out long fileLen)
        {
            var bytes = File.ReadAllBytes(filePath);
            fixed (byte* bb = bytes)
            {
                fileLen = bytes.LongLength;
                return new vstd.MD5(bb, (ulong)bytes.LongLength);
            }
        }

        public static bool GetFileDataFromDB(
            in IMongoCollection<BsonDocument> dbCollect,
            in vstd.MD5 md5,
            in long fileSize,
            out FileData result)
        {
            var v = dbCollect.Find(
                Builders<BsonDocument>.Filter.And(
                        Builders<BsonDocument>.Filter.Eq("md5", md5.ToString()),
                        Builders<BsonDocument>.Filter.Eq("size", fileSize),
                        Builders<BsonDocument>.Filter.Exists("path"),
                        Builders<BsonDocument>.Filter.Exists("guid")
                    ));
            if (v.CountDocuments() != 1)
            {
                result = new FileData();
                return false;
            }
            BsonDocument doc = v.First();

            result = new FileData
            {
                fileMD5 = md5,
                fileSize = fileSize,
                fileGuid = new vstd.Guid(doc.GetValue("guid").AsString),
                filePath = doc.GetValue("path").AsString
            };

            return true;
        }

        public static bool GetFileDataFromDB(
            in IMongoCollection<BsonDocument> dbCollect,
            in vstd.Guid guid,
            out FileData result)
        {
            string guidStr = guid.ToString();
            var v = dbCollect.Find(
               Builders<BsonDocument>.Filter.And(
                        Builders<BsonDocument>.Filter.Eq("guid", guidStr),
                        Builders<BsonDocument>.Filter.Exists("md5"),
                        Builders<BsonDocument>.Filter.Exists("path"),
                        Builders<BsonDocument>.Filter.Exists("size")
                   ));
            if (v.CountDocuments() != 1)
            {
                result = new FileData();
                return false;
            }
            BsonDocument doc = v.First();
            result = new FileData
            {
                fileGuid = guid,
                filePath = doc.GetValue("path").AsString,
                fileSize = doc.GetValue("size").AsInt64,
                fileMD5 = vstd.MD5.FromString(doc.GetValue("md5").AsString)
            };
            return true;
        }
        public static bool GetFileDataFromDB(
            in IMongoCollection<BsonDocument> dbCollect,
            in string filePath,
            out FileData result)
        {
            var v = dbCollect.Find(
              Builders<BsonDocument>.Filter.And(
                        Builders<BsonDocument>.Filter.Eq("path", filePath),
                        Builders<BsonDocument>.Filter.Exists("md5"),
                        Builders<BsonDocument>.Filter.Exists("guid"),
                        Builders<BsonDocument>.Filter.Exists("size")
                  ));
            if (v.CountDocuments() != 1)
            {
                result = new FileData();
                return false;
            }
            BsonDocument doc = v.First();
            result = new FileData
            {
                fileGuid = new vstd.Guid(doc.GetValue("guid").AsString),
                filePath = filePath,
                fileSize = doc.GetValue("size").AsInt64,
                fileMD5 = vstd.MD5.FromString(doc.GetValue("md5").AsString)
            };
            return true;
        }

        public static void AddFileDataToDB(
            in IMongoCollection<BsonDocument> dbCollect,
            in vstd.Guid guid,
            in string filePath)
        {
            long fileSize;
            var md5 = CalcFileMD5(filePath, out fileSize);
            Dictionary<string, object> dict = new Dictionary<string, object>();
            dict.Add("guid", guid.ToString());
            dict.Add("path", filePath);
            dict.Add("size", fileSize);
            dict.Add("md5", md5.ToString());
            dbCollect.InsertOne(new BsonDocument(dict));
        }
        public static void ResetFilePath(
            in IMongoCollection<BsonDocument> dbCollect,
            in vstd.MD5 tarFileMD5,
            in long tarFileSize,
            in string oldPath,
            in string newPath)
        {
            dbCollect.UpdateOne(
                Builders<BsonDocument>.Filter.And(
                     Builders<BsonDocument>.Filter.Eq("md5", tarFileMD5.ToString()),
                     Builders<BsonDocument>.Filter.Eq("size", tarFileSize),
                     Builders<BsonDocument>.Filter.Eq("path", oldPath)
                    ),
                    Builders<BsonDocument>.Update.Set("path", newPath)
                );
        }
        public static void UpdateFileData(
            in IMongoCollection<BsonDocument> dbCollect,
            in string filePath)
        {
            long fileSize;
            var newMD5 = CalcFileMD5(filePath, out fileSize);
            dbCollect.UpdateOne(
                Builders<BsonDocument>.Filter.And(
                    Builders<BsonDocument>.Filter.Eq("path", filePath),
                    Builders<BsonDocument>.Filter.Exists("size"),
                    Builders<BsonDocument>.Filter.Exists("md5")),
                Builders<BsonDocument>.Update.Combine(
                    Builders<BsonDocument>.Update.Set("size", fileSize),
                    Builders<BsonDocument>.Update.Set("md5", newMD5.ToString())
             ));
        }
    }
}
