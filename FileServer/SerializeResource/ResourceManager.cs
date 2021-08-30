using System;
using System.Collections;
using System.Collections.Generic;
using MongoDB.Bson;
using MongoDB.Driver;
namespace FileServer
{

    public static class ResourceManager
    {
        public static Dictionary<vstd.Guid, SerializeResource> openedFiles = new Dictionary<vstd.Guid, SerializeResource>();
        public static bool CreateFile(
            in MongoDatabase db,
            in string name,
            in vstd.Guid guid,
            in vstd.Guid userID)
        {
            string guidStr = guid.ToCompressString();
            string userStr = userID.ToCompressString();
            var dbCollect = db.serCollect;
            lock (dbCollect)
            {
                var v = dbCollect.Find(Builders<BsonDocument>.Filter.Eq("id", guidStr));
                if (v.FirstOrDefault() == null)
                {
                    List<BsonElement> elements = new List<BsonElement>();
                    elements.Add(new BsonElement("id", guidStr));
                    elements.Add(new BsonElement("name", name));
                    elements.Add(new BsonElement("is_writing", true));
                    elements.Add(new BsonElement("is_stable", false));
                    elements.Add(new BsonElement("write_user", userStr));
                    dbCollect.InsertOne(new BsonDocument(elements));
                    return true;
                }
                return false;

            }
        }
        public static bool DeleteFile(
            in MongoDatabase db,
            in vstd.Guid guid)
        {
            string guidStr = guid.ToCompressString();
            var dbCollect = db.serCollect;

            var v = dbCollect.FindOneAndDelete(
                Builders<BsonDocument>.Filter.And(
                Builders<BsonDocument>.Filter.Eq("id", guidStr),
                Builders<BsonDocument>.Filter.Eq("is_writing", false)));
            return (v != null);
        }
        public static bool OpenReadonlyFile(
            in MongoDatabase db,
            in vstd.Guid guid,
            in bool onlyStable)
        {
            var dbCollect = db.serCollect;
            var guidStr = guid.ToCompressString();
            FilterDefinition<BsonDocument> filter;
            if (onlyStable) filter = Builders<BsonDocument>.Filter.Eq("id", guidStr);
            else filter =
                    Builders<BsonDocument>.Filter.And(
                    Builders<BsonDocument>.Filter.Eq("id", guidStr),
                    Builders<BsonDocument>.Filter.Eq("is_stable", true));
            var v = dbCollect.Find(
                filter);
            bool result = (v != null);
            if (result)
            {

            }
            return result;

        }
        public static bool OpenWritableFile(
            in MongoDatabase db,
            in vstd.Guid userID,
            in vstd.Guid guid)
        {
            var dbCollect = db.serCollect;
            var guidStr = guid.ToCompressString();
            var userStr = userID.ToCompressString();
            var v = dbCollect.FindOneAndUpdate(
                Builders<BsonDocument>.Filter.And(
                    Builders<BsonDocument>.Filter.Eq("id", guidStr),
                    Builders<BsonDocument>.Filter.Eq("is_writing", false)
                ),

                Builders<BsonDocument>.Update.Combine(
                    Builders<BsonDocument>.Update.Set("is_writing", true),
                    Builders<BsonDocument>.Update.Set("write_user", userStr)
                )
            );
            return (v != null);
        }

        public static bool CloseWritableFile(
           in MongoDatabase db,
            in vstd.Guid userID,
            in vstd.Guid guid)
        {
            var dbCollect = db.serCollect;
            var guidStr = guid.ToCompressString();
            var userStr = userID.ToCompressString();
            var v = dbCollect.FindOneAndUpdate(
                Builders<BsonDocument>.Filter.And(
                    Builders<BsonDocument>.Filter.Eq("id", guidStr),
                    Builders<BsonDocument>.Filter.Eq("is_writing", true),
                    Builders<BsonDocument>.Filter.Eq("write_user", userStr)
                ),
                Builders<BsonDocument>.Update.Set("is_writing", false)

            );
            return v != null;
        }
    }
}