# wayland学习

[The Wayland Protocol（自译中文版）](https://wayland.axionl.me/1.Introduction/index.html)这本书能快速深入理解Wayland的概念、设计和实现，并为您提供构建自行构建Wayland客户端和服务端所需的工具。通过简单的例子，快速去理解wayland的设计和原理。

下面是从第四节开始的wayland和server的例子。

## 编译



[例子链接](https://wayland.axionl.me/4.The_Wayland_display/1.Creating_a_display.html)


```bash
# ubuntu编译用到的命令
cc -o server server.c -lwayland-server
cc -o client client.c -lwayland-client
cc -o client client.c xdg-shell-protocol.c -lwayland-client -lrt
cc -o globals  globals.c -lwayland-client
cc -o draw-blank-rect draw-blank-rect.c  -lwayland-client 
cc -o  draw-grid  draw-grid.c xdg-shell-protocol.c -lwayland-client -lrt

> ubuntu上wayland server使用这个简单的例子还不能绘制，不能输出interface，这个在安装weston后，
在tty(ctrl+alt+[f2-f7])环境下执行globals就能输出interface的打印和绘制窗口了。

## 使用weston

Wayland是一套display server(Wayland compositor)与client间的通信协议，而Weston是Wayland compositor的参考实现。

### 安装运行weston
```bash
sudo apt install weston

# ctl+alt+f4切换到tty4，也可以用`sudo chvt 4`来切换，数字4代表tty4
weston-launch
```
执行`weston-launch`后，切换到tty4，可以看到weston完整的桌面，点击左上角的按钮后会出现新终端界面。这时候执行前面编译的globals程序，就会输出interface的打印，在ubuntu上没有输出，是因为我们的server代码还很简单，没有注册这些interface，而使用weston server就能得到了完整的输出了。

```bash
interface: 'wl_compositor', version: 4, name: 1
interface: 'wl_subcompositor', version: 1, name: 2
interface: 'wp_viewporter', version: 1, name: 3
interface: 'zxdg_output_manager_v1', version: 2, name: 4
interface: 'wp_presentation', version: 1, name: 5
interface: 'zwp_relative_pointer_manager_v1', version: 1, name: 6
interface: 'zwp_pointer_constraints_v1', version: 1, name: 7
interface: 'zwp_input_timestamps_manager_v1', version: 1, name: 8
interface: 'wl_data_device_manager', version: 3, name: 9
interface: 'wl_shm', version: 1, name: 10
interface: 'wl_drm', version: 2, name: 11
interface: 'wl_seat', version: 7, name: 12
interface: 'zwp_linux_dmabuf_v1', version: 3, name: 13
interface: 'weston_direct_display_v1', version: 1, name: 14
interface: 'zwp_linux_explicit_synchronization_v1', version: 2, name: 15
interface: 'weston_content_protection', version: 1, name: 16
interface: 'wl_output', version: 3, name: 17
interface: 'wl_output', version: 3, name: 18
interface: 'zwp_input_panel_v1', version: 1, name: 19
interface: 'zwp_input_method_v1', version: 1, name: 20
interface: 'zwp_text_input_manager_v1', version: 1, name: 21
interface: 'xdg_wm_base', version: 1, name: 22
interface: 'zxdg_shell_v6', version: 1, name: 23
interface: 'wl_shell', version: 1, name: 24
interface: 'weston_desktop_shell', version: 1, name: 25
interface: 'weston_screenshooter', version: 1, name: 26

```

## 生成xdg-shell-protocol代码
[显示640x480 像素的棋盘格](https://wayland.axionl.me/7.XDG_shell_basics/3.Extended_example_code.html)


```bash
$ sudo apt install wayland-protocols
$ wayland-scanner private-code   < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml   > xdg-shell-protocol.c
$ wayland-scanner client-header   < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml   > xdg-shell-client-protocol.h

cc -o  draw-grid  draw-grid.c xdg-shell-protocol.c -lwayland-client -lrt

```

```


## 关于tty

TTY来源于Teletypewriter，翻译过来就是电传打字机。Teletypewriter和普通打字机的区别在于，Teletypewriter连接到通信设备以发送所打印的文字信息，电传打字机使人类能够通过电线更快地通信，这就是“TTY”这个词最初产生的时间。

随着技术的进一步发展，实体的电传打字机进行了“虚拟化”。因此不再需要物理、机械的TTY，而是虚拟的电子TTY。早期的计算机甚至没有视频屏幕，东西印在纸上，而不是显示在屏幕上。因此，你会看到“打印”一词的使用，而不是“显示”。随着技术的进步，显示器才被添加到终端中，可以称它们为“物理”终端。然后，这些演变成软件模拟终端，并具有增强的能力和功能。这就是所谓的“终端模拟器”。

Linux中的TTY是一个抽象设备。有时它指的是物理输入设备，如串行端口，有时它指的是允许用户与系统交互的虚拟TTY。

可以在大多数发行版上使用以下键盘快捷键来获取TTY屏幕：
```bash
CTRL + ALT + F1 – 锁定屏幕
CTRL + ALT + F2 – 桌面环境
CTRL + ALT + F3 – TTY3
CTRL + ALT + F4 – TTY4
CTRL + ALT + F5 – TT5
CTRL + ALT + F6 – TTY6
```


# 编译y4 arm版本
unset LD_LIBRARY_PATH
source /opt/mina/1.0/environment-setup-armv7at2hf-neon-minamllib32-linux-gnueabi

$CC -o server server.c -lwayland-server
$CC -o client client.c -lwayland-client
$CC -o client client.c xdg-shell-protocol.c -lwayland-client -lrt
$CC -o globals  globals.c -lwayland-client
$CC -o draw-blank-rect draw-blank-rect.c  -lwayland-client 
$CC -o  draw-grid.c draw-grid.c xdg-shell-protocol.c -lwayland-client -lrt
```

---

## 参考链接<br>

[The Wayland Protocol（自译中文版）](https://wayland.axionl.me/)<br>

[第一个黑框框](https://memorytoco.gitbook.io/wayland-gui/first-blackbox)<br>

[Wayland入门1：运行测试程序](https://zhuanlan.zhihu.com/p/427978971)
