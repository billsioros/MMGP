{
    "targets":
    [
        {
            "target_name": "group",
            "sources":
            [
                "main.cpp",
                "../../cpp/src/wrapper.cpp",
                "../../cpp/src/timewindow.cpp",
                "../../cpp/src/vector2.cpp",
                "../../cpp/src/manager.cpp",
                "../../cpp/src/log.cpp"
            ],
            "include_dirs":
            [
                "../../cpp/include/",
                "../../cpp/SQLiteCpp/include/"
            ],
            "cflags!": [ "-fno-exceptions" ],
            "cflags": [ "-W", "-g3", "-std=c++17" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "cflags_cc": [ "-W", "-g3", "-std=c++17" ],
            'link_settings': {
                'libraries': [
                    '-L lib/libsqlcpp.so'
                ]
            }
        }
    ]
}
