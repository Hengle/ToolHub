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
        static object locker = new object();

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
            BsonDocument doc;
            lock (locker)
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

                doc = v.First();
            }
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
            BsonDocument doc;
            lock (locker)
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
                doc = v.First();
            }
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
            BsonDocument doc;
            lock (locker)
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
                doc = v.First();
            }
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
            bool isUpdate;
            lock (locker)
            {
                var result = dbCollect.FindOneAndUpdate(
                Builders<BsonDocument>.Filter.And(
                Builders<BsonDocument>.Filter.Eq("path", filePath)
                ),
                Builders<BsonDocument>.Update.Combine(
                    Builders<BsonDocument>.Update.Set("md5", md5.ToString()),
                    Builders<BsonDocument>.Update.Set("size", bytes.LongLength)
                ));
                string guidStr = fileGuid.ToString();
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
                Network.FileUtility.WriteFile(filePath, bytes);
                File.WriteAllBytes(filePath + ".meta", metaBytes);
            }
            return isUpdate;
        }
        public static void ResetFilePath(
            in IMongoCollection<BsonDocument> dbCollect,
            in vstd.Guid guid,
            in string newPath)
        {
            lock (locker)
            {
                var doc = dbCollect.FindOneAndUpdate(
                Builders<BsonDocument>.Filter.And(
                    Builders<BsonDocument>.Filter.Eq("guid", guid.ToString())
                    ),
                    Builders<BsonDocument>.Update.Set("path", newPath)
                );
                var oldPath = doc.GetValue("path").AsString;
                File.Move(oldPath, newPath);
            }
        }

        public static void DeleteFile(
            in IMongoCollection<BsonDocument> dbCollect,
            in string guid)
        {
            lock (locker)
            {
                var doc = dbCollect.FindOneAndDelete(
                Builders<BsonDocument>.Filter.And(
                    Builders<BsonDocument>.Filter.Eq("guid", guid),
                    Builders<BsonDocument>.Filter.Exists("md5"),
                    Builders<BsonDocument>.Filter.Exists("path"),
                    Builders<BsonDocument>.Filter.Exists("size")));
                if (doc != null)
                    File.Delete(doc.GetValue("path").AsString);
            }
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="dbCollect"></param>
        /// <param name="guid"></param>
        /// <param name="fileBytes"></param>
        /// <param name="fileMetaBytes"></param>
        /// <returns>0: success, 1: empty guid, 2: error path</returns>
        public static int TryDownload(
            in IMongoCollection<BsonDocument> dbCollect,
            in vstd.Guid guid,
            out string filePath,
            out byte[] fileBytes,
            out byte[] fileMetaBytes)
        {
            lock (locker)
            {
                var doc = dbCollect.Find(
                Builders<BsonDocument>.Filter.Eq("guid", guid.ToString()));
                var bson = doc.First();
                if (bson == null)
                {
                    filePath = null;
                    fileBytes = null;
                    fileMetaBytes = null;
                    return 1;
                }
                filePath = bson.GetValue("path").AsString;
                fileBytes = File.ReadAllBytes(filePath);
                fileMetaBytes = File.ReadAllBytes(filePath + ".meta");
                return (fileBytes == null || fileMetaBytes == null) ? 2 : 0;
            }
        }
    }
}
