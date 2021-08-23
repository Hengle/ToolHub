using System;
using vstd;
namespace Network
{
    class Program
    {
        static void PrintObj(SerializeObject obj)
        {
            var rootNode = obj.GetRootNode();
            var subArray = rootNode.GetArray("array");
            for (var ite = subArray.Begin(); !subArray.End(ite); subArray.GetNext(ref ite))
            {
                Console.WriteLine(subArray.GetInt(ite));
            }
            var subDict = rootNode.GetDict("dict");
            for (var ite = subDict.Begin(); !subDict.End(ite); subDict.GetNext(ref ite))
            {
                Console.WriteLine(subDict.GetKeyString(ite) + ": " + subDict.GetString(ite));
            }
        }
        static void Main(string[] args)
        {
            {
                Console.WriteLine("Serialize Database: ");
                using (SerializeObject obj = new SerializeObject(true))
                {

                    {
                        var rootNode = obj.GetRootNode();
                        var subArray = obj.CreateNewArray();
                        var subDict = obj.CreateNewDict();
                        rootNode.SetDict("dict", subDict.Move());
                        rootNode.SetArray("array", subArray.Move());
                        subArray.AddInt(5);
                        subArray.AddInt(6);
                        subArray.AddInt(7);
                        subDict.SetString("fuck", "xxx");
                        subDict.SetString("fuck1", "yyy");
                        subDict.SetString("fuck2", "zzz");
                        obj.SerializeToFile("TestSerialize.bytes");
                    }
                    PrintObj(obj);

                }

            }
            {
                Console.WriteLine("DeSerialized Database: \n\n");

                //Read from file
                using (SerializeObject obj = new SerializeObject(true))
                {
                    obj.ReadFromFile("TestSerialize.bytes");
                    PrintObj(obj);

                }
            }
        }
    }
}
