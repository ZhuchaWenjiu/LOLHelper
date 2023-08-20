#include "curl/curl.h"
#include <string>
#include <json/json.h>
#include <iostream>
#include <vector>

class LOLHelper
{
private:
    std::string urlStart;
    std::string port;
    std::string token;
    Json::Reader reader;
    std::string ConvId;
    int pastFlag;
    int playerNum;
    Json::Value value;
    std::vector<std::string> teamAccountIds;
    std::vector<std::string> teamNickNames;
    std::vector<std::string> teamPuuiDs;
    std::vector<float> teamSocker;
    CURL* curl;
    struct curl_slist* headers = NULL;
    void httpInit();
    void heardInit();
public:
    LOLHelper(std::string port, std::string token);
    ~LOLHelper();
    void printPerm();
    void getQQ();
    void getPlayerNickName();
    void getUrl(std::string url, std::string& res);
    void postString(std::string url, std::string str);
    void deleteItem(std::string url);
    int getClineState();
    std::string getConversationId();
    void getPlayersAccountId(std::string convId);
    Json::Value readJsonFromString(std::string str);
    void getTeamPlayersInfo(int flag);
    void getUserInfoFormAccountId(int Id);
    bool isStateChanged();
    void confirm();
    void post(std::string url);
    void sendMessage(std::string str);
    void sendTeamPlayersInfo();
    void getPlayerSockerByPuuid(int id);
    void printJson(Json::Value data);
};



