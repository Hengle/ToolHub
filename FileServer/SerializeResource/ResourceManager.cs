using System;
using System.Collections;
using System.Collections.Generic;
using MongoDB.Bson;
using MongoDB.Driver;
namespace FileServer
{

    public static class ResourceManager
    {
        public class OpenFile
        {
            public SerializeResource resource;
            public vstd.Guid writingUser;
        }
        public static Dictionary<vstd.Guid, OpenFile> openedFiles = new Dictionary<vstd.Guid, OpenFile>();
        public static bool CreateFile(
            in MongoDatabase db,
            in string name,
            in long version,
            out vstd.Guid guid)
        {
            guid = new vstd.Guid(true);
            string guidStr = guid.ToString();
            var dbCollect = db.serCollect;
            lock (dbCollect)
            {
                var v = dbCollect.Find(Builders<BsonDocument>.Filter.Eq("id", guidStr));
                if (v.FirstOrDefault() == null)
                {
                    List<BsonElement> elements = new List<BsonElement>();
                    elements.Add(new BsonElement("id", guidStr));
                    elements.Add(new BsonElement("name", name));
                    elements.Add(new BsonElement("version", version));
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
            string guidStr = guid.ToString();
            var dbCollect = db.serCollect;

            var v = dbCollect.FindOneAndDelete(Builders<BsonDocument>.Filter.Eq("id", guidStr));
            return (v != null);
        }
        public static bool FindFile(
            in MongoDatabase db,
            in vstd.Guid guid)
        {
            var dbCollect = db.serCollect;
            var guidStr = guid.ToString();
            FilterDefinition<BsonDocument> filter = Builders<BsonDocument>.Filter.Eq("id", guidStr);
            var v = dbCollect.Find(
                filter).FirstOrDefault();
            bool result = (v != null);
            return result;

        }
        public static vstd.Guid FindFile(
            in MongoDatabase db,
            in string name,
            in long version)
        {
            var dbCollect = db.serCollect;
            FilterDefinition<BsonDocument> filter =
                Builders<BsonDocument>.Filter.And(
                Builders<BsonDocument>.Filter.Eq("name", name),
                Builders<BsonDocument>.Filter.Eq("version", version));
            var v = dbCollect.Find(
                filter).FirstOrDefault();
            if(v == null)
            {
                return new vstd.Guid(0, 0);
            }
            return new vstd.Guid(v.GetValue("id").AsString);
        }
    }
}