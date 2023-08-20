#include "LOLHelper.hpp"

LOLHelper::LOLHelper(std::string inPort, std::string inToken){
    port = inPort;
    urlStart = "https://127.0.0.1:" + inPort;
    token = inToken;
    pastFlag = 0;
    teamAccountIds.resize(10);
    teamNickNames.resize(10);
    teamPuuiDs.resize(10);
    teamSocker.resize(10);
    httpInit();
    heardInit();
}

LOLHelper::~LOLHelper() {};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* user_data) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(user_data);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}



void LOLHelper::heardInit(){
    std::string h1 = "Authorization: Basic " + token;
    std::string h2 = "Host: " + urlStart;
    std::cout << h1 << std::endl;
    std::cout << h2 << std::endl;
    headers = curl_slist_append(headers, h1.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    // headers = curl_slist_append(headers, h2.c_str());
}

void LOLHelper::httpInit(){
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

void LOLHelper::printPerm(){
    std::cout << "port : " << port << std::endl;
    std::cout << "token : " << token << std::endl;
    std::cout << "urlStart : " << urlStart << std::endl;
}

void LOLHelper::getQQ(){
    std::string url = urlStart + "/lol-login/v1/session";
    std::string res;
    // std::cout << "url : " << url << std::endl;
    getUrl(url, res);
    auto jsonRes = readJsonFromString(res);
    std::string qq = jsonRes.get("username",NULL).asString();
    // std::cout << qq;
}

Json::Value LOLHelper::readJsonFromString(std::string str){
    //1.创建工厂对象
    Json::CharReaderBuilder ReaderBuilder;
    ReaderBuilder["emitUTF8"] = true;//utf8支持,不加这句,utf8的中文字符会编程\uxxx
    //2.通过工厂对象创建 json阅读器对象
    std::unique_ptr<Json::CharReader> charread(ReaderBuilder.newCharReader());
    //3.创建json对象
    Json::Value root;
    //4.把字符串转变为json对象,数据写入root
    std::string strerr;
    bool isok = charread->parse(str.c_str(),str.c_str()+str.size(),&root,&strerr);
    if(!isok || strerr.size() != 0){
        std::cout <<"json解析出错";
    }
    //5.返回有数据的json对象,这个json对象已经能用了
    return root;
}

void LOLHelper::getUrl(std::string url, std::string& res){
    CURLcode stat;

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);  
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); // 以上两条设置为不验证SSL证书
    stat = curl_easy_perform(curl);
    if (stat != CURLE_OK) {
        std::cerr << "ERROR !!!! curl error" << curl_easy_strerror(stat) << std::endl;
    }
}

void LOLHelper::getPlayerNickName(){
    std::string url = urlStart + "/lol-summoner/v1/current-summoner";
    std::string res;
    getUrl(url, res);
    auto jsonRes = readJsonFromString(res);
    std::string nickName = jsonRes.get("displayName",NULL).asString();
    std::cout << nickName;
}

std::string LOLHelper::getConversationId(){
    std::string url = urlStart + "/lol-chat/v1/conversations";
    std::string res;
    getUrl(url, res);
    auto jsonRes = readJsonFromString(res);
    std::string convId = jsonRes[0]["id"].asString();
    ConvId = convId;
    return convId;
}

void LOLHelper::getPlayersAccountId(std::string convId ){
    std::string url = urlStart + "/lol-chat/v1/conversations/" + convId + "/messages" ;
    std::string res;
    getUrl(url, res);
    auto jsonRes = readJsonFromString(res);
    for (int i = 0; i < jsonRes.size(); i++){
        teamAccountIds[i] = jsonRes[i]["fromId"].asString();
    }
    playerNum = jsonRes.size();
    std::cout  << "player number " << playerNum << std::endl;
}

int LOLHelper::getClineState(){
    std::string url = urlStart + "/lol-gameflow/v1/gameflow-phase";
    std::string res;
    getUrl(url, res); // 将客户端状态赋值给res
    if(res.compare("\"ReadyCheck\"") == 0)
        return 1; // 秒确认

    if( res.compare("\"ChampSelect\"") == 0)
        return 2; // 英雄选择

    if( res.compare("\"InProgress\"") ==0 )
        return 3; // 游戏中

    return 0; // 其他情况
}

/**
 * @brief 获得队伍中队员信息
 * @param flag  0 为友方队伍， 1 为敌方队伍
 */
void LOLHelper::getTeamPlayersInfo(int flag){
    int startId = flag * 5; 

    std::string convId = getConversationId();

    getPlayersAccountId(convId);

    for (size_t i = startId; i < startId + playerNum; i++){
        getUserInfoFormAccountId(i);
    }
}

/**
 * @brief 由AccountId获取用户的昵称和Puuid
 * @param Id player的AccountId在teamAccountIds中的索引
 */
void LOLHelper::getUserInfoFormAccountId(int Id){
    std::string accId = teamAccountIds[Id];
    std::string url = urlStart + "/lol-summoner/v1/summoners/" + accId;
    std::string res;
    getUrl(url, res);
    auto jsonRes = readJsonFromString(res);
    teamNickNames[Id] = jsonRes["displayName"].asString();
    teamPuuiDs[Id] = jsonRes["puuid"].asString();
    // std::cout << "Nick " <<  teamNickNames[Id] << std::endl;
    // std::cout << "Puuid " <<  teamPuuiDs[Id] << std::endl;
    getPlayerSockerByPuuid(Id);
}

void LOLHelper::getPlayerSockerByPuuid(int id){
    std::string puuid = teamPuuiDs[id];
    std::string url = urlStart + "/lol-match-history/v1/products/lol/" + puuid + "/matches";
    std::string res;
    getUrl(url, res);
    auto jsonRes = readJsonFromString(res);
    Json::Value games = jsonRes["games"]["games"]; // 获得近 21 条游戏数据
    int count = 0;
    float sumDGrate = 0.0f; // 总金币输出转化率
    int sumVisionScore = 0;
    float sumKDA = 0.0f;
    for (int i = 0; i < games.size(); i++ ){
        Json::Value game = games[i];
        if(count == 5)
            break;
        // std::cout << game["gameType"];
        // std::cout << game["mapId"];
        if( game["gameType"].asString().compare("\"MATCHED_GAME\"")  
           && game["mapId"].asInt() == 11 ){
            // std::cout << game["stats"].asString();
            auto info = game["participants"][0]["stats"];
            int assists = info["assists"].asInt(); // 助攻
            int death = info["deaths"].asInt(); // 死亡
            int kill = info["kills"].asInt(); // 击杀
            int damageToTurret = info["damageDealtToTurrets"].asInt(); // 对塔伤害
            int damageToPlayer= info["totalDamageDealtToChampions"].asInt(); // 对英雄伤害
            bool isWin = info["win"].asBool(); // 是否获胜
            bool isEarlySurrender = info["causedEarlySurrender"].asBool(); // 是否提前投降
            int damageToObj = info["damageDealtToObjectives"].asInt(); // 兵营伤害
            int visionScore = info["visionScore"].asInt(); // 视野分
            sumVisionScore += visionScore;
            int gold = info["goldEarned"].asInt();
            
            // 对英雄伤害 + 对塔伤害 * 2 + 对兵营伤害 * 2  =  局内有效伤害
            int damage = damageToObj*2 + damageToTurret * 2 + damageToPlayer;

            // 金币转化率 = damage / gold
            float DGrate = (float)damage / (float)gold;
            sumDGrate += DGrate;
            // kda
            if(death == 0)
                death = 1;
            float KDA = (float)(assists + kill ) / (float)death;
            sumKDA += KDA;
            count++;
        }
        
    }
    
    std::string msg = teamNickNames[id] + "的近5场场均KDA为 " + std::to_string(sumKDA / (float)count) + " 伤害金币转化率为 " 
    + std::to_string(sumDGrate / (float)count) + "视野得分为 " + std::to_string(sumVisionScore / count) + " 视野分取整";
    sendMessage(teamNickNames[id]);
    sendMessage("近5场场均KDA为 " + std::to_string(sumKDA / (float)count));
    sendMessage("伤害经济转化率为" + std::to_string(sumDGrate / (float)count));
    sendMessage("视野得分为 " + std::to_string(sumVisionScore / count));
    // sendMessage(msg);  
}

bool LOLHelper::isStateChanged(){
    int curStatFlag = getClineState();
    if(pastFlag == curStatFlag){
        return false;
    }
    pastFlag = curStatFlag;
    return true;
}

void LOLHelper::confirm(){
    std::string url = urlStart + "/lol-matchmaking/v1/ready-check/accept";
    post(url);
}

void LOLHelper::postString(std::string url, std::string str){
    CURLcode stat;
    CURL *pCurl = curl_easy_init();
    std::string msg = "{ \"body\": \""+ str + "\",\"type\": \"chat\"}";
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers); // 添加请求头
    curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
    // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0);  
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 0); // 以上两条设置为不验证SSL证书

    curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, msg.c_str());  
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, msg.size());
    stat = curl_easy_perform(pCurl);
    if (stat != CURLE_OK) {
        std::cerr << "ERROR !!!! curl error" << curl_easy_strerror(stat) << std::endl;
    }
    curl_easy_cleanup(pCurl);
}

void LOLHelper::post(std::string url){
    CURLcode stat;

    CURL *pCurl = curl_easy_init();
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers); // 添加请求头
    curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0);  
    curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 0); // 以上两条设置为不验证SSL证书
    curl_easy_setopt(pCurl, CURLOPT_POST, 1L);

    // stat = curl_easy_perform(pCurl);
    // std::cout << stat;
    // if (stat != CURLE_OK) {
    //     std::cerr << "ERROR !!!! curl error" << curl_easy_strerror(stat) << std::endl;
    // }
    curl_easy_cleanup(pCurl);
}

void LOLHelper::sendMessage(std::string str){
    std::string url = urlStart + "/lol-chat/v1/conversations/" + ConvId + "/messages";
    postString(url, str);
}

/**
 * @brief 发送队友信息
 */
void LOLHelper::sendTeamPlayersInfo(){
    getTeamPlayersInfo(0); // 获取队友信息

}
