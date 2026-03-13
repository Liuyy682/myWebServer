# myWebServer

## 项目简介
myWebServer 是一个简单易用的Web服务器项目，旨在帮助开发者快速启动和运行小型Web服务。它是一个学习和实验的平台，适合新手和有经验的开发者使用。

## 安装说明
要在本地安装和运行myWebServer，请按照以下步骤操作：

1. **克隆项目**：
   ```
   git clone https://github.com/yourusername/myWebServer.git
   ```
   
2. **进入项目目录**：
   ```
   cd myWebServer
   ```
   
3. **编译项目**：
   ``` 
   cmake -S . -B build
   cmake --build build
   ```

## 使用指南
编译完成后，可以运行以下命令来启动服务器：

```
./build/server
```

服务器启动后，可以在浏览器中访问 `http://localhost:8080`。

## 使用 webbench 压测

### 1) 安装 webbench（项目内）
```bash
bash scripts/install_webbench.sh
```

或者使用 CMake 目标：
```bash
cmake --build build --target webbench_install
```

### 2) 启动服务
```bash
./build/server
```

### 3) 执行压测
默认参数：并发 `100`、时长 `10s`、URL `http://127.0.0.1:8080/`。

```bash
bash scripts/run_webbench.sh
```

或者使用 CMake 目标：
```bash
cmake --build build --target webbench_run
```

### 4) 自定义压测参数
可通过环境变量覆盖默认值：

- `WEBBENCH_C`：并发数（例如 `200`）
- `WEBBENCH_T`：测试时长秒数（例如 `30`）
- `WEBBENCH_URL`：压测地址（例如 `http://127.0.0.1:8080/index.html`）
- `WEBBENCH_HTTP_OPT`：HTTP版本参数（`-1`/`-2`/`-9`）
- `WEBBENCH_FORCE_RELOAD`：是否开启强制刷新（`1` 开启）

示例：
```bash
WEBBENCH_C=300 WEBBENCH_T=20 WEBBENCH_FORCE_RELOAD=1 \
WEBBENCH_URL=http://127.0.0.1:8080/index.html \
bash scripts/run_webbench.sh
```

## 贡献说明
欢迎对myWebServer进行贡献！如果您有想法或者遇到了问题，请随时在GitHub上开Issue或者提交Pull Request。

## 许可
该项目采用MIT许可证，详情请参见LICENSE文件。