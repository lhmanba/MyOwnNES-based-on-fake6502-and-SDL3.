# 基于little6502和SDL的NES模拟器开发日志

## day 1

目标：看到一个可缩放的256×240测试画面。

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

### 问题

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