using System;
using System.Collections.Generic;
using Native;
using MongoDB.Driver;
using MongoDB.Bson;

namespace AssetDatabase
{
    unsafe class Program
    {
       static  void UpdateFileDict(Dictionary<string, object> dict, in vstd.Guid guid, string name)
        {
            dict.Clear();
            dict.Add("guid", guid.ToString());
            dict.Add("name", name);
        }
        static void Main(string[] args)
        {
            Memory.vengine_init_malloc();
            MongoClient dbClient = new MongoClient("mongodb://localhost:27017");
            var db = dbClient.GetDatabase("fuck");
            Console.WriteLine("Input '0' to drop last");
            Console.WriteLine("Input '1' to add new");
            string s = Console.ReadLine();
            switch (s)
            {
                case "0":
                    db.DropCollection("root");
                    return;
                case "1":
                    {
                        db.CreateCollection("root");
                        var localRoot = db.GetCollection<BsonDocument>("root");
                        List<BsonDocument> docs = new List<BsonDocument>();
                        Dictionary<string, object> dict = new Dictionary<string, object>();
                        UpdateFileDict(dict, new vstd.Guid(true), "fuckOne");
                        docs.Add(new BsonDocument(dict));
                        UpdateFileDict(dict, new vstd.Guid(true), "shitOne");
                        docs.Add(new BsonDocument(dict));
                        localRoot.InsertMany(docs);
                    }
                    return;
            }
            var rootDoc = db.GetCollection<BsonDocument>("root");
            //Add Code
            /*             
              
             */
            var findResult = rootDoc.Find(Builders<BsonDocument>.Filter.Eq("guid", "4D530EE0C0CD312000B7C55E1903B88D"));
            var docCount = findResult.CountDocuments();
            Console.WriteLine("find count: " + docCount);
            //findResult.
            if (docCount > 0)
            {
                var docs = findResult.ToList();
                foreach (var doc in docs)
                {

                    Console.WriteLine(doc.GetValue("guid").AsString);
                    Console.WriteLine(doc.GetValue("name").AsString);

                }
                //rootDoc.Find()
            }
            Console.WriteLine("Finished!");

            /*
            var command = new BsonDocument { { "dbstats", 1 } };
            var result = db.RunCommand<BsonDocument>(command);
            var cars = db.GetCollection<BsonDocument>("cars");
            //db.CreateCollection("cars",);*/
        }
    }
}
