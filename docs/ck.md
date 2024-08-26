# nebula_pro项目
地图项目后端，用于收集终端设备，并可视化至地图界面。另外，支持与其他服务进行数据交互，从而形成一个数据交换中心。
编译注意事项：
将Qt目录换到自己的Qt目录，env的环境变量换成自己的目录，SetQt5Dir.cmake中修改Qt5_SDK_DIR为自己的盘符。
1、先执行"./env.sh"初始化环境，
2、然后执行"./bootstrap.sh"脚本生成makefile文件
3、最后"./build.sh r"进行编译

在relay_module_North模块下写代码，编译后的结果保存在G:\Desktop\code\nebula_pro\release\Nebula.RelayModule.exe可执行文件中
需要从其他地方拷贝ssh.dll、pthreadVC3.dll两个目录到当前路径


# urd360项目
1、执行"./make -r"命令会自动编译当前目录
2、执行"./publish"命令会打tag标签上传至gitlab,在执行过程中，会调用python脚本文件zentao.py来发布版本至禅道中。

 find_package(OpenCV 3 REQUIRED)命令执行流程：
 1、进入搜索路径：/usr/local/share/OpenCV
 2、找到OpenCVConfig.cmake文本，并执行该文件
 3、定义OpenCV_INCLUDE_DIR、OpenCV_LIBS、OpenCV_DIR等变量
 4、在Cmakelists.txt文件中引用完变量后完成编译

# find_package
find_package命令有两种工作模式，分别是module模式与config模式，这两种工作模式决定了不同的搜索包路径。
find_package本质上就是一个搜包命令，通过一些特定的规则找到<package_name>Config.cmake包配置文件，执行该配置文件，从而定义一些变量，通过这些变量可以准确定位库的头文件与库文件，从而完成编译。
**REQUIRED**：可选字段，表示一定要找到该包，找不到则立即停掉整个CMake。如果不指定该字段，那么CMake会继续执行。
**COMPONENTS**：可选字段，表示查找的包中必须要找到的组件，如果任何一个找不到就算失败，类似于required，导致CMake停止执行。

## module模式(默认)
module模式默认只有两个查找路径，CMAKE_MODULE_PATH和CMake安装路径下的Modules目录。
搜包路径次序为：CMAKE_MODULE_PATH, CMAKE_ROOT
先在CMAKE_MODULE_PATH变量对应的路径中查找。如果路径为空，或者路径中查找失败，则在CMake安装目录(即CMAKE_ROOT变量)下的Modules目录下(/usr/share/cmake-3.10)查找。
## config模式
只有在find_config中制定了config或no_module等关键字，或者module模式查找

