using System;
using System.Collections.Generic;
using Native;
using MongoDB.Driver;
using MongoDB.Bson;
using System.IO;
using Network;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.InteropServices;

namespace FileServer
{
    [Network.RPC(Network.RPCLayer.All)]
    public static class RPCTest
    {
        public static void PrintString(object str)
        {
            string s = (string)str;
            Console.WriteLine("From Remote: " + s);
        }
    }
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

        static void RunSerialization()
        {
            RPCReflector.LoadRPCFunctor(System.Reflection.Assembly.GetExecutingAssembly(), RPCLayer.All);
            Console.WriteLine("input y for server, other for client: ");
            string s = Console.ReadLine();
            bool isServer = (s == "y");
            if (isServer)
            {
                using (RPCSocket serverSocket = new RPCSocket(2021))
                {
                    Console.WriteLine("Connect Success!");
                    while (true)
                    {
                        string inputStr = Console.ReadLine();
                        if (inputStr == "exit") break;
                        serverSocket.CallRemoteFunction(
                            "RPCTest",
                            "PrintString",
                            inputStr);
                    }
                }
            }
            else
            {
                using (RPCSocket clientSocket = new RPCSocket("127.0.0.1", 2021))
                {
                    Console.WriteLine("Connect Success!");
                    System.Threading.Thread.Sleep(new TimeSpan(1, 0, 0, 0, 0));

                }
            }
        }
        static void Main(string[] args)
        {
            RPCReflector.LoadRPCFunctor(System.Reflection.Assembly.GetExecutingAssembly(), RPCLayer.All);
        }
    }
}
