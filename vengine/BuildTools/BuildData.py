Proj = None
SubProj = [
    {
        "Name": "VEngine_DLL",
        "PlaceHolder": "../Build/#.lib",
        "RemoveHeader": 1,
        "Dependices": [
        ],
        "IncludePaths": ["."],
        "PreProcessor": {
            "Debug": [
                "_DEBUG",
                "COMMON_DLL_PROJECT",
                "_CONSOLE",
                "_CRT_SECURE_NO_WARNINGS",
                "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"
            ],
            "Release": [
                "NDEBUG",
                "COMMON_DLL_PROJECT",
                "_CONSOLE",
                "_CRT_SECURE_NO_WARNINGS",
                "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"
            ]
        }
    },
    {
        "Name": "VEngine_Compute",
        "Dependices": [
        ],
        "IncludePaths": [".", "./VectorCompute"],
        "PreProcessor": {
            "Debug": [
                "_DEBUG",
                "VENGINE_COMPUTE_PROJECT",
                "_CONSOLE",
                "_CRT_SECURE_NO_WARNINGS",
                "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"
            ],
            "Release": [
                "NDEBUG",
                "VENGINE_COMPUTE_PROJECT",
                "_CONSOLE",
                "_CRT_SECURE_NO_WARNINGS",
                "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"
            ]
        }
    }

]

ContainedFiles = {
    'cpp': 'ClCompile',
    'c': 'ClCompile',
    'cc': 'ClCompile',
    'cxx': 'ClCompile',
    'h': 'ClInclude',
    'hpp': 'ClInclude',
    'lib': 'Library'
}
IgnoreFolders = {
    "x64": 1,
    "x86": 1,
    ".vs": 1,
    "Shaders": 1,
    "Doc": 1,
    "Project": 1,
    "Build/ShaderCompileResult": 1,
    "Build/Data": 1,
    "Build/Resource": 1,
    "MonoInclude": 1,
    "lib": 1,
    'BuildTools': 1
}
IgnoreFile = {
    "VEngine_Compute.lib": 1,
    "program.inf": 1
}

CopyFilePaths = []
