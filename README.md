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

## 使用 wrk 压测

### 1) 安装 wrk（项目内）
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
默认参数：线程 `4`、连接 `400`、时长 `10s`、URL `http://127.0.0.1:8080/`。

> 注：为兼容旧命令，脚本名仍为 `run_webbench.sh`/`install_webbench.sh`，但内部已切换为 `wrk`。

```bash
bash scripts/run_webbench.sh
```

或者使用 CMake 目标：
```bash
cmake --build build --target webbench_run
```

### 4) 自定义压测参数
可通过环境变量覆盖默认值：

- `WRK_THREADS`：线程数（例如 `4`）
- `WRK_CONNECTIONS`：并发连接数（例如 `400`）
- `WRK_DURATION`：测试时长（例如 `10s`）
- `WRK_URL`：压测地址（例如 `http://127.0.0.1:8080/index.html`）
- `WRK_TIMEOUT`：请求超时（例如 `2s`）

兼容旧变量（仍可用）：

- `WEBBENCH_C` -> `WRK_CONNECTIONS`
- `WEBBENCH_T` -> `WRK_DURATION`（自动补 `s`）
- `WEBBENCH_URL` -> `WRK_URL`

示例：
```bash
WRK_THREADS=4 WRK_CONNECTIONS=600 WRK_DURATION=20s \
WRK_URL=http://127.0.0.1:8080/index.html \
bash scripts/run_webbench.sh
```

### 5) 常见问题：`wrk` 安装失败

如果脚本提示缺少依赖，可安装：

```bash
sudo apt-get update
sudo apt-get install -y unzip
```

## 贡献说明
欢迎对myWebServer进行贡献！如果您有想法或者遇到了问题，请随时在GitHub上开Issue或者提交Pull Request。

## 许可
该项目采用MIT许可证，详情请参见LICENSE文件。