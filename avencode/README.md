gcc编译命令如下：

```bash

export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu/:$LD_LIBRARY_PATH

gcc -o encode encode.c -lavcodec -lavutil

./encode -i 256x144-yuv420p.yuv -s 256x144 -o out.mp4

```

# pkg-config libpng16

```bash
$ pkg-config libpng16 --cflags --libs
-I/usr/local/include/libpng16 -L/usr/local/lib/x86_64-linux-gnu -lpng16 -lz
```
所以增加-L参数应该就可以：

```bash
gcc -o encode encode.c -lavcodec -lavutil -L/usr/lib/x86_64-linux-gnu/ -lpng16
```

```bash
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_set_IHDR@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_get_io_ptr@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_set_longjmp_fn@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_set_PLTE@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_write_info@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_create_info_struct@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_write_image@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_write_end@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_create_write_struct@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_set_write_fn@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_set_tRNS@PNG16_0'
/usr/bin/ld: /lib/x86_64-linux-gnu/libzvbi.so.0: undefined reference to `png_set_gAMA@PNG16_0'
```

指定-L/usr/lib/x86_64-linux-gnu/ -lpng16可以解决上面的问题。