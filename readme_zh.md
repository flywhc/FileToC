# 文件至PROGMEM C字符串转换器

## 引言
此脚本将文件转换为可存储于PROGMEM中的C字符串形式，可以用于在Arduino ESP8266工程中构建网站服务器。

通常Arduino项目通过SPIFFS或LittleFS文件系统在Flash存储器中保存文件。而此脚本提供了一种替代方案，能将文件转化为C语言的PROGMEM字符串，可以把资源文件直接整合进项目编译流程，无需另外采用SPIFFS或LittleFS文件上传工具。

对于搭载小容量Flash芯片的设备而言，其SPIFFS或LittleFS文件系统的空间可能极为有限，此时将文件存储于PROGMEM中，有助于容纳更大规模的文件。存入PROGMEM的文件会与Flash中的程序代码（标记为ICACHE_FLASH_ATTR，亦即“Sketch”）共用存储空间。PROGMEM所能存储文件的总量可能少于或多于SPIFFS或LittleFS文件系统，具体视开发板的Flash布局而定。关于这部分的详细信息，请参考[Arduino ESP8266文件系统](https://arduino-esp8266.readthedocs.io/zh_CN/latest/filesystem.html)文档。

Flash中程序代码的容量大小，可在Arduino的构建日志中查询到。例如：
```
. Code in flash (default, ICACHE_FLASH_ATTR), used 315092 / 1048576 bytes (30%)
║   SEGMENT  BYTES    DESCRIPTION
╚══ IROM     315092   code in flash
```

## 特性
* 保留文件名和文件路径，以便Web服务器能够以相对路径返回文件。
* 压缩HTML、CSS和JavaScript文件以减小体积。
* 提供Web服务器请求处理器以支持生成的PROGMEM文件。
  
## 支持平台
* Arduino ESP8266。已在ESP-01S上测试。

* 其他平台可能稍作修改后也可运行。

## 使用方法
### 准备Python环境

导入Conda环境配置文件environment.yml来配置Python环境。

例如：
```
conda env create -f environment.yml -n c:\projects\FileToCString\.conda
conda activate c:\projects\FileToCString\.conda
```

请记得把`c:\projects\FileToCString` 替换成你的项目目录。

### 运行脚本把文件转换成C文件
 
 在你的项目目录中，执行下面的命令把目录中的文件转换成C语言代码：
```
file_to_c.py <directory> [-r] [-c]
    directory: 包含资源文件的目录
    -r: 包含子目录里的文件
    -c：压缩HTML, JS和CSS文件.
```
例如
```
cd samples\esp8266
python ..\..\file_to_c.py webdata -r -c
```

### 了解输出的文件
脚本会在执行命令的同一目录下生成`<directory>.h`以及`<directory>.c`文件。HTML、CSS 和 JavaScript 文件在作为字符串存储之前会进行压缩，而其他文件则以二进制格式存储。这些输出可以直接包含在项目的代码库中。

所有文件都存储在一个 `ProgmemFileInformation` 结构的数组中：
```
extern const ProgmemFileInformation progmemFiles[];
```
`ProgmemFileInformation`在`ProgmemFileInformation.h`中定义：
```
typedef struct ProgmemFileInformationStruct
{
    const char * file_path;
    const char * file_content; // PROGMEM
    const int file_length;
    const char * content_type;
} ProgmemFileInformation;
```

* `file_path`: 相对于`<directory>`的路径。
* `file_content`: PROGMEM C字符串格式的文件内容。
* `file_length`: 文件长度。
* `content_type`: 用于web服务器的文件MIME类型。


### 在Web服务器中使用生成的C文件
创建一个`ProgmemWebRequest`对象并在构造中使用`progmemFiles`作为输入.
```
const char *ignoredDirectories[] = {"/api", "/cgi-bin"};
webserver.addHandler(new ProgmemWebRequest(progmemFiles, ignoredDirectories));
```
可选的，你可以在`/api`或`/cgi-bin`路径下添加动态网页或者API接口。

注意：`on()`方法应在`addHandler()`之前，否则addHandler()方法将不起作用。
```
webserver.on("/api/toggle_led", toggleLed); // 演示如何开关板上LED灯的API接口
```

注意：默认根请求会读取`<directory>`目录下的`index.htm`。

### 构建并上传至设备
在编译之前，请务必在 `config.h` 文件中更新 `WIFI_SSID` 和 `WIFI_PASSWORD` 以匹配您的无线网络设置。

如果一切顺利，您可以通过访问 http://espserver 来打开web服务器，并通过点击绿色按钮来控制板上内置的LED灯。

![screenshot](images/demo.png)

## License
此脚本使用MIT许可证。

但是如果你在GPL项目中使用它生成的文件则必须以GPL许可证发布。