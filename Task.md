# 项目详情
 - 项目名称：Vaelen
 - 项目描述：这是一个自研Linux内核 无桌面 仅CLI界面

# 项目功能
 - 自研Linux内核 + Linux系统
 - 仅CLI界面
 - 无桌面环境
 - 自带Linux常用命令(如ls, cd, mkdir, rm,apt-get等等)

# 项目实现
 - 自研Linux内核
 - 自研Linux系统

# 注意事项：
1. 项目需打包成ISO文件，用于引导安装
2. 项目必须在物理机中运行，不支持虚拟机中运行
3. 物理机测试配置：
   - 测试配置：
      - CPU：Intel Inside CORE I5
      - 内存：8GB
      - 硬盘：495GB
    *Tips: 此为测试机配置，后续需要支持主流配置

# 项目打包后文件：
 - Vaelen-linux-version.iso  -- 正式版本ISO文件(前期不需要打包)
 - Vaelen-linux-version-TEST.iso  -- 测试版本ISO文件

正式版ISO文件须在用户通知发布正式版本时打包，打包后文件名需为Vaelen-linux-version.iso
默认打包测试版本ISO文件，文件名需为Vaelen-linux-version-TEST.iso