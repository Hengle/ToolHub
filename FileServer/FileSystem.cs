using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Native;
using MongoDB.Driver;
using MongoDB.Bson;

namespace FileServer
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
        static readonly string FILE_FOLDER = "Files/";
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
        /// <summary>
        /// 
        /// </summary>
        /// <param name="dbCollect"></param>
        /// <param name="filePath"></param>
        /// <param name="bytes"></param>
        /// <returns> update(true) or add(false) </returns>
        public static bool UpdateFileDataToDB(
            in IMongoCollection<BsonDocument> dbCollect,
            in vstd.Guid fileGuid,
            in string filePath,
            in byte[] bytes,
            in byte[] metaBytes)
        {
            vstd.MD5 md5;
            fixed (byte* b = bytes)
            {
                md5 = new vstd.MD5(b, (ulong)bytes.LongLength);
            }
            var result = dbCollect.FindOneAndUpdate(
            Builders<BsonDocument>.Filter.And(
                Builders<BsonDocument>.Filter.Eq("path", filePath)
                ),
             Builders<BsonDocument>.Update.Combine(
                Builders<BsonDocument>.Update.Set("md5", md5.ToString()),
                Builders<BsonDocument>.Update.Set("size", bytes.LongLength)
                )
            );
            string guidStr = fileGuid.ToString();
            bool isUpdate;
            if (result == null)
            {
                Dictionary<string, object> dict = new Dictionary<string, object>();
                dict.Add("guid", guidStr);
                dict.Add("path", filePath);
                dict.Add("size", bytes.LongLength);
                dict.Add("md5", md5.ToString());
                dbCollect.InsertOne(new BsonDocument(dict));
                isUpdate = false;
            }
            else
            {
                guidStr = result.GetValue("guid").AsString;
                isUpdate = true;
            }
            string path = FILE_FOLDER + guidStr;
            File.WriteAllBytes(path, bytes);
            File.WriteAllBytes(path + ".meta", metaBytes);
            return isUpdate;
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

        public static void DeleteFile(
            in IMongoCollection<BsonDocument> dbCollect,
            in string guid)
        {
            var doc = dbCollect.FindOneAndDelete(
                Builders<BsonDocument>.Filter.And(
                    Builders<BsonDocument>.Filter.Eq("guid", guid),
                    Builders<BsonDocument>.Filter.Exists("md5"),
                    Builders<BsonDocument>.Filter.Exists("path"),
                    Builders<BsonDocument>.Filter.Exists("size")));
            if (doc != null)
                File.Delete(FILE_FOLDER + doc.GetValue("guid").AsString);
        }
    }
}
