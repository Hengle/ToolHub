using System;
using System.Collections.Generic;
using Native;
using MongoDB.Driver;
using MongoDB.Bson;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;

namespace AssetDatabase
{
    unsafe class Program
    {
        static void UpdateFileDict(Dictionary<string, object> dict, in vstd.Guid guid, string name)
        {
            dict.Clear();
            dict.Add("guid", guid.ToString());
            dict.Add("name", name);
        }

        static void RunDB()
        {
            MongoClient dbClient = new MongoClient("mongodb://localhost:27017");
            //dbClient.DropDatabase("fuck");

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
            var findResult = rootDoc.Find(Builders<BsonDocument>.Filter.Empty);
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
        [System.Serializable]
        struct CustomPerson
        {
            public int a;
            public double b;
            public string c;
            public List<int> values;
        }
        static void RunSerialization()
        {
            MemoryStream memorystream = new MemoryStream();
            BinaryFormatter bf = new BinaryFormatter();
            CustomPerson person = new CustomPerson
            {
                a = 324,
                b = 1378.1654,
                c = "fuck!",
                values = new List<int>()
            };
            person.values.Add(5);
            person.values.Add(43);
            person.values.Add(2);
            person.values.Add(56);
            bf.Serialize(memorystream, person);
            memorystream.Position = 0;
            CustomPerson newPerson = (CustomPerson)bf.Deserialize(memorystream);
            Console.WriteLine(newPerson.a);
            Console.WriteLine(newPerson.b);
            Console.WriteLine(newPerson.c);
            foreach(var i in newPerson.values)
            {
                Console.WriteLine(i);
            }

        }
        static void Main(string[] args)
        {
            Memory.vengine_init_malloc();
            RunSerialization();
        }
    }
}
