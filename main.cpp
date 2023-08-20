#include "main.hpp"
#include "LOLHelper.hpp"



int main(){
    std::string port, token;
    system("chcp 65001"); // 设置命令行编码
    getPortAndToken(port, token); // 获得 port 和 token
    auto tokenD = encodeBase64("riot:"+token); // base64 编码后的token
    LOLHelper helper(port, tokenD); // 初始化LOLHelper
    helper.printPerm();
    helper.getQQ();
    helper.getPlayerNickName();
    int fastClick;
    std::cout << "\n是否启动快速确认 0 for no , 1 for yes \n";
    std::cin >> fastClick;
    while (true){
        if(helper.isStateChanged()){
            int stat = helper.getClineState();
            if( stat == 2 ){
                helper.sendTeamPlayersInfo();
                std::string masg = "数据由近五场得出";
                helper.sendMessage(masg);
            }
            else if (stat == 3 ){
                std::cout << "in game ! \n";
            }
            else if (stat == 1 && fastClick == 1){
                std::cout << " confirm \n";
                helper.confirm();
            }
        }
    }
    
    system("pause");
    return 0;
}