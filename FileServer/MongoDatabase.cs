using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MongoDB.Driver;
using MongoDB.Bson;

namespace FileServer
{
    public class MongoDatabase
    {
        MongoClient dbClient;
        private IMongoDatabase fileDB;
        public IMongoDatabase resManagerDB { get; private set; }
        public IMongoCollection<BsonDocument> fileCollect { get; private set; }
        public IMongoCollection<BsonDocument> serCollect { get; private set; }
        public MongoDatabase()
        {
            dbClient = new MongoClient("mongodb://localhost:27017");
            fileDB = dbClient.GetDatabase("file");
            resManagerDB = dbClient.GetDatabase("res_manager");
            fileCollect = fileDB.GetCollection<BsonDocument>("file_content");
            serCollect = resManagerDB.GetCollection<BsonDocument>("ser_content");
        }

    }
}
