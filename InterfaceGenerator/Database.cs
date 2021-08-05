using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
[GenerateCPP("VEngine_Unity.dll", "Database_codegen.hpp", "Database_codegen.cs", "toolhub")]
class Database
{
    public Database() { }
    public IntPtr GetResourceDatabase() { return default; }
    public IntPtr GetSceneDatabase(int sceneIndex) { return default; }
}

