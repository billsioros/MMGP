{
    "targets":
    [
        {
            "target_name": "tsp",
            "sources":
            [
                "../TSP/src/main.cpp",
                "../cpp/SQLiteCpp/src/Backup.cpp",
                "../cpp/SQLiteCpp/src/Column.cpp",
                "../cpp/SQLiteCpp/src/Database.cpp",
                "../cpp/SQLiteCpp/src/Exception.cpp",
                "../cpp/SQLiteCpp/src/Statement.cpp",
                "../cpp/SQLiteCpp/src/Transaction.cpp"
            ],
            "include_dirs":
            [
                "../cpp/include/",
                "../cpp/SQLiteCpp/include/",
                "../cpp/nlohmann/"
            ],
            "cflags!": [ "-fno-exceptions" ],
            "cflags": [ "-W", "-g3", "-std=c++14" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "cflags_cc": [ "-W", "-g3", "-std=c++14" ]
        }
    ]
}
