# LOLHelper
发送队友信息
## 已实现功能
- 查询队友战绩
- 计算队友得分
- 发送队友信息

==秒同意还没弄，try catch 也没做，有时快点就会捕捉空内容导致jsoncpp报错，直接重启就行==

必须使用管理员模式启动，不然无法获得LOL端口信息

如果想自己编译使用以下命令

g++ .\main.cpp .\LOLHelper.cpp -o main -lwsock32  -lcurl -ljsoncpp 
后面的三个库需要自行安装， libcurl 和 jsoncpp, 第一个套接字应该自带吧
