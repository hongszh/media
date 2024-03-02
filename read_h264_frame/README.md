## read h264 frame

`read_h264_frames1.c`和`read_h264_frames2.c`这两个代码都是解析文件名为`256x144.h264`的H264流文件，然后打印每一个frame的大小，输出frame大小信息，和文件名为`videoframes.xml`的xml统计信息对比，区别就是这两个C代码中`Frame 0`和`Frame 1`的输出分别是SPS和PPS的4个字节的startcode开始的帧，而ffmpeg命令生成的xml文件中第一针是包含了SPS和PPS。

编译：

```bash
gcc -g -o read_h264_frame read_h264_frames1.c
gcc -g -o read_h264_frame read_h264_frames2.c
```

vscode中已经配置调试命令和参数，编译后直接可以进入vscode debug模式：

```bash
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "C/C++ Runner: Debug Session",
      "type": "cppdbg",
      "request": "launch",
      "args": [
        "${workspaceFolder}/256x144.h264"
      ],
      "stopAtEntry": false,
      "externalConsole": false,
      "cwd": "${workspaceFolder}",
      "program": "${workspaceFolder}/read_h264_frame",
      "MIMode": "gdb",
      "miDebuggerPath": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ]
    }
  ]
}
```

## 生成xml文件的命令
> 生成xml后手动删掉一些没用的列：

```bash
ffprobe -show_frames -select_streams v -of xml 256x144.h264 > videoframes.xml
```