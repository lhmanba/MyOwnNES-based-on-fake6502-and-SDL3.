# 基于little6502和SDL的NES模拟器开发日志

## day 1

### 目标：

看到一个可缩放的256×240测试画面。

当天必须理解：framebuffer只是一段按行排列的像素内存；SDL纹理负责把宿主内存送给 GPU，它不是 PPU。先用人工测试图验证显示链，可以确保以后黑屏时优先检查模拟核心，而不是怀疑窗口系统。

- [x] 创建目录和 CMake工程。
- [x] 加入 SDL3并成功构建空窗口。
- [x] 创建256×240 ARGB8888流式纹理。
- [x] 用棋盘格或彩条填充 framebuffer。
- [x] 处理关闭窗口事件。
- [x] 设置最近邻缩放和逻辑分辨率。

### 学习到的点

#### cmake基础

- project(... LANGUAGES C)表明这是一个C项目

- set(CMAKE_C_STANDARD 11)使用C11

- add_subdirectory(third_party/SDL)表明SDL是一个子Cmake项目，需要一起编译

- add_executable(minines src/main.c)表示最终生成的是minines.exe

- target_link_libraries(minines PRIVATE SDL3::SDL3)链接SDL3

- 配置命令

  - cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

  -S指定CMake源码目录-B指定构建目录-G指定构建工具-DCMAKE_BUILD_TYPE=Debug调试模式

- 构建命令

  - cmake --build build

    \-\-build指定构建规则文件夹

#### nes相关

- framebuffer

  Framebuffer 本质上就是一块内存，用来暂时保存“一整帧画面中每个像素应该是什么颜色”。

​	`framebuffer[y * 256 + x] = color;`把(x,y)设置为color

#### SDL相关

- SDL窗口创建流程：
  window->render->texture

  texture缩放模式->render逻辑呈现模式

- 小巧思：

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  RenderClear()；

  设置背景为黑色，保证填充不满时有黑边

### 成果图

![image-20260916163837705](./%E5%9F%BA%E4%BA%8Elittle6502%E5%92%8CSDL%E7%9A%84NES%E6%A8%A1%E6%8B%9F%E5%99%A8%E5%BC%80%E5%8F%91%E6%97%A5%E5%BF%97.assets/image-20260916163837705.png)

### 遇到的问题

- 为什么要采用临近缩放？
  - 不用平滑插值，避免像素边缘被线性插值模糊

- 启动时报错：找不到 SDL3.dll

  - 说明程序用的是动态链接，还需要SDL3.dll

  - 在CMakeLists.txt中末尾添加：

    ```
    add_custom_command(TARGET minines POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:minines>
    )
    ```

    自动复制SDL3.dll到minines目录

### 补充实验

- 四象图

  更改fill_test_pattern![image-20260916224505580](./%E5%9F%BA%E4%BA%8Elittle6502%E5%92%8CSDL%E7%9A%84NES%E6%A8%A1%E6%8B%9F%E5%99%A8%E5%BC%80%E5%8F%91%E6%97%A5%E5%BF%97.assets/image-20260916224505580.png)

```
static void fill_test_pattern(void)
{
    for (int y = 0; y < NES_HEIGHT; ++y) {
        for (int x = 0; x < NES_WIDTH; ++x) {

            uint32_t color;

            if (x < NES_WIDTH / 2 && y < NES_HEIGHT / 2) {
                color = 0xFFFF0000;   // 红
            }
            else if (x >= NES_WIDTH / 2 && y < NES_HEIGHT / 2) {
                color = 0xFF00FF00;   // 绿
            }
            else if (x < NES_WIDTH / 2 && y >= NES_HEIGHT / 2) {
                color = 0xFF0000FF;   // 蓝
            }
            else {
                color = 0xFFFFFFFF;   // 白
            }

            nes_framebuffer[y * NES_WIDTH + x] = color;
        }
    }
}
```

- 移动的绿色方块
  ![image-20260916233419678](./%E5%9F%BA%E4%BA%8Elittle6502%E5%92%8CSDL%E7%9A%84NES%E6%A8%A1%E6%8B%9F%E5%99%A8%E5%BC%80%E5%8F%91%E6%97%A5%E5%BF%97.assets/image-20260916233419678.png)

```
static void fill_test_pattern(uint16_t square_x)
{
    for (int y = 0; y < NES_HEIGHT; ++y) {
        for (int x = 0; x < NES_WIDTH; ++x) {
            nes_framebuffer[y*NES_WIDTH + x] = 0xFFFFFFFF;
        }
    }
    for (int y = 100; y < 116; ++y)
    {
        for (int x = square_x; x < square_x + 16; ++x)
        {
            nes_framebuffer[y * NES_WIDTH + x] = 0xFF00FF00;
        }
    }
}
```

## day2

### 目标：

正确打印 NROM的元数据并能读取复位向量。

当天必须理解：`.nes`文件不是可以从头顺序执行的程序。16字节头部描述后面 PRG/CHR数据的布局；Mapper决定 CPU和 PPU地址怎样转换为文件内偏移；Reset向量位于虚拟 CPU地址 `$FFFC/$FFFD`，必须经过 Mapper读取。

- [x] 实现安全的16字节 iNES解析。
- [x] 处理 Trainer。
- [x] 加载 PRG和 CHR；CHR为0时创建 CHR RAM。
- [x] 拒绝非 Mapper 0。
- [x] 实现 NROM-128与 NROM-256 CPU映射。
- [x] 实现 CHR ROM/RAM的 PPU映射。
- [x] 打印 `$FFFC/$FFFD`形成的 Reset向量。

### 学习到的点

- nes文件
  ![image-20260917081854522](./%E5%9F%BA%E4%BA%8Elittle6502%E5%92%8CSDL%E7%9A%84NES%E6%A8%A1%E6%8B%9F%E5%99%A8%E5%BC%80%E5%8F%91%E6%97%A5%E5%BF%97.assets/image-20260917081854522.png)

  - PRG ROM
    CPU执行游戏逻辑等的机器码
  - CHR ROM
    Tile图案，8x8像素，注意，NES一像素为2bit

  - .nes`uint_8 h[16]`

  | Offset | 意义                                                       |
  | ------ | ---------------------------------------------------------- |
  | `0~3`  | `N E S 0x1A`     用来确定这是一个nes文件                   |
  | `4`    | PRG ROM 数量，每块 16 KiB    若h[4]=2,表明PRG有2*16=32字节 |
  | `5`    | CHR ROM 数量，每块 8 KiB                                   |
  | `6`    | Mirroring、Trainer、Mapper低4位等                          |
  | `7`    | Mapper高4位等                                              |
  | `8~15` | 其他/保留                                                  |

- mapper
  NES 本体的 CPU 和 PPU 能直接访问的地址空间有限，但很多游戏的 ROM 比这个空间大，所以卡带需要一种机制，把 ROM 的不同部分“切换”进当前可访问的地址范围。这个机制就是 Mapper。

- h[6]&0x08=1 -> Four-screen
  h[6]&0x1=1 -> Vertical
  h[6]&0x1=0 -> Horizontal

- CHR有两种情况：
  - chr_size>0：卡带自带chrrom
  - chr_size=0：卡带无chrrom 需要创建ram单独控制图块
  
- NEScpu是6502系列cpu，地址为16位，从\$0000~$FFFF

  - 我们分配\$8000~$FFFF给mapper0的PRG rom

### 成果图

![image-20260917225025648](./%E5%9F%BA%E4%BA%8Elittle6502%E5%92%8CSDL%E7%9A%84NES%E6%A8%A1%E6%8B%9F%E5%99%A8%E5%BC%80%E5%8F%91%E6%97%A5%E5%BF%97.assets/image-20260917225025648.png)

### 问题

- 为什么不能把nes头转换为结构体而是直接解析字节？
- 为什么32kib的prg不需要镜像而16kib的prg需要镜像？
  - 因为 CPU 仍然可能去读高地址，尤其是Reset Vector 地址。

## day 3

### 目标：

CPU从 Reset向量持续执行，并可输出 trace。

当天必须理解：CPU只产生地址读写，不直接认识 PPU和 ROM；总线是设备选择器。CPU执行结果和周期同等重要，trace中的“第一个差异”通常比最后的崩溃位置更接近根因。

- [x] 加入 Fake6502及其许可证。
- [x] 启用 NES CPU/禁用有效 BCD。
- [x] 实现2 KB RAM和镜像。
- [x] 实现 PPU寄存器占位访问。
- [x] 映射 `$8000-$FFFF`到 Mapper 0。
- [x] 调用 `reset6502()`并逐指令执行。
- [x] 加入可开关 trace。

![image-20260918211519988](./%E5%9F%BA%E4%BA%8Elittle6502%E5%92%8CSDL%E7%9A%84NES%E6%A8%A1%E6%8B%9F%E5%99%A8%E5%BC%80%E5%8F%91%E6%97%A5%E5%BF%97.assets/image-20260918211519988.png)

### 学到的东西

- 6502cpu的状态
  A   Accumulator
  X   X index register
  Y   Y index register
  SP  Stack Pointer
  P   Processor Status
  PC  Program Counter
  - 其中P是一个8位状态寄存器
    - bit 7  N   Negative
      bit 6  V   Overflow
      bit 5  常量位
      bit 4  B   Break
      bit 3  D   Decimal
      bit 2  I   Interrupt Disable
      bit 1  Z   Zero
      bit 0  C   Carry

## day4

### 目标：

CPU能通过 `$2000-$2007`正确读写 PPU内存。

当天必须理解：CPU地址空间和 PPU地址空间彼此独立；`$2006/$2007`只是 CPU访问 PPU地址空间的窗口。寄存器读取可能有副作用，Name Table镜像来自物理显存地址线连接，不是画面水平/垂直翻转。

- [x] 实现 PPU地址14位镜像。
- [x] 实现水平/垂直 Name Table镜像。
- [x] 实现 Palette镜像。
- [x] 实现 `$2000-$2007`寄存器基本副作用。
- [x] 实现 `v/t/fine_x/write_toggle/read_buffer`。
- [x] 实现 OAM及 `$4014` DMA。
- [x] 用日志确认 ROM向 Name Table和 Palette写入。

## day5

### 目标：

把 PPU内存解释为可见画面。

当天必须理解：NES背景不是位图，而是 Name Table指定 Tile、Pattern Table提供2位像素、Attribute Table选择调色板组、Palette RAM选择最终颜色。精灵来自另一条 OAM管线，最终才与背景按透明度和优先级合成。

- [x] 添加来源明确的64色 ARGB表。
- [x] 实现 Pattern Table位平面解码。
- [x] 实现 Name Table和 Attribute Table背景渲染。
- [x] 实现水平/垂直滚屏的基础版本。
- [ ] 实现8×8精灵、透明、翻转和优先级。
- [x] 将 framebuffer上传 SDL纹理。

## day6

### 目标：

画面会更新，键盘能操作。

当天必须理解：SDL帧率只是宿主展示速度；游戏的“每帧”来自 PPU进入 VBlank并触发 NMI。手柄也不是8个可随机读取的寄存器，而是先锁存快照，再按固定顺序逐位读出的串行设备。

- [x] 实现 scanline/dot计数。
- [x] 在241行 dot 1设置 VBlank并按 PPUCTRL触发 NMI。
- [x] 在 pre-render行清状态。
- [x] CPU每周期推进3个 PPU dot。
- [x] 实现 `$4016` strobe和移位读取。
- [x] 映射方向键、Z/X、Enter、右Shift。
- [x] 加入60 Hz节拍。
