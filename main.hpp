#include <regex>        //c++ 11 正则表达式
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef unsigned char     uint8;
typedef unsigned long    uint32;

static uint8 alphabet_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8 reverse_map[] =
{
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
     255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
     255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
};

#pragma comment(lib, "ws2_32.lib")   //网络库文件

/**
 * @brief 将 执行cmd 命令的结果返回 给 result
 * @param cmd 命令  
 * @param result  命令返回的结果
 * @return 
 */
int execmd(char* cmd,char* result) {
	char buffer[128];                         //定义缓冲区                        
	FILE* pipe = _popen(cmd, "r");            //打开管道，并执行命令 
	if (!pipe)
		return 0;                      //返回0表示运行失败 
 
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe)){             //将管道输出到result中 
			strcat(result,buffer);
		}
	}
	_pclose(pipe);                            //关闭管道 
	return 1;                                 //返回1表示运行成功 
}

/**
 * @brief 获得 LOLCline 的本地端口和访问服务器的token
 * @param port 端口
 * @param token  token
 */
void getPortAndToken(std::string &port, std::string &token){
    char* getLOLClinePortAndTockenCmd= "WMIC PROCESS WHERE name=\"LeagueClientUx.exe\" GET commandline";

    char result[1024 * 9] = "";                   //定义存放结果的字符串数组 
    std::string fullText;

    std::regex getPort("--app-port=(.*?)\""); // 获得 port 的 正则 
    std::regex getToken("--remoting-auth-token=(.*?)\""); // 获得 token 的 正则
    std::smatch match; // 匹配后的结果

	execmd(getLOLClinePortAndTockenCmd, result); // 执行命令，获得LOLCline的信息
    
    // std::cout << "result " << result << "\n\n"; // 获得 LOLCline 的 app-port ， tocken
    fullText = result;
    if( regex_search(fullText, match, getPort) ){
        port = match[1];
    }
    else{
        std::cout << "LOL cline not running or this program not running as admin \n";
    }

    if( regex_search(fullText, match, getToken) ){
        token = match[1];
    }
}

uint32 base64_encode(const uint8 *text, uint32 text_len, uint8 *encode)
{
    uint32 i, j;
    for (i = 0, j = 0; i+3 <= text_len; i+=3)
    {
        encode[j++] = alphabet_map[text[i]>>2];                             //取出第一个字符的前6位并找出对应的结果字符
        encode[j++] = alphabet_map[((text[i]<<4)&0x30)|(text[i+1]>>4)];     //将第一个字符的后2位与第二个字符的前4位进行组合并找到对应的结果字符
        encode[j++] = alphabet_map[((text[i+1]<<2)&0x3c)|(text[i+2]>>6)];   //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符
        encode[j++] = alphabet_map[text[i+2]&0x3f];                         //取出第三个字符的后6位并找出结果字符
    }

    if (i < text_len)
    {
        uint32 tail = text_len - i;
        if (tail == 1)
        {
            encode[j++] = alphabet_map[text[i]>>2];
            encode[j++] = alphabet_map[(text[i]<<4)&0x30];
            encode[j++] = '=';
            encode[j++] = '=';
        }
        else //tail==2
        {
            encode[j++] = alphabet_map[text[i]>>2];
            encode[j++] = alphabet_map[((text[i]<<4)&0x30)|(text[i+1]>>4)];
            encode[j++] = alphabet_map[(text[i+1]<<2)&0x3c];
            encode[j++] = '=';
        }
    }
    return j;
}

uint32 base64_decode(const uint8 *code, uint32 code_len, uint8 *plain)
{
    assert((code_len&0x03) == 0);  //如果它的条件返回错误，则终止程序执行。4的倍数。

    uint32 i, j = 0;
    uint8 quad[4];
    for (i = 0; i < code_len; i+=4)
    {
        for (uint32 k = 0; k < 4; k++)
        {
            quad[k] = reverse_map[code[i+k]];//分组，每组四个分别依次转换为base64表内的十进制数
        }

        assert(quad[0]<64 && quad[1]<64);

        plain[j++] = (quad[0]<<2)|(quad[1]>>4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的前2位进行组合

        if (quad[2] >= 64)
            break;
        else if (quad[3] >= 64)
        {
            plain[j++] = (quad[1]<<4)|(quad[2]>>2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应base64表的十进制数的前4位进行组合
            break;
        }
        else
        {
            plain[j++] = (quad[1]<<4)|(quad[2]>>2);
            plain[j++] = (quad[2]<<6)|quad[3];//取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
        }
    }
    return j;
}

/**
 * @brief 返回 string 类型的 base64编码后的字符串
 * @param str 输入字符串
 * @return 输出 base64编码后的字符串
 */
std::string encodeBase64(std::string str){
    // std::cout << str << "\n";
    char s[256];
    strcpy(s, str.c_str());
    uint8 *text = (uint8 *)s; // token 转换 uint8
    uint32 text_len = (uint32)strlen((char *)text);
    uint8  buffer[4096];
    uint32 size = base64_encode(text, text_len, buffer);
    buffer[size] = 0;
    std::string res((char *)buffer);
    return res;
}