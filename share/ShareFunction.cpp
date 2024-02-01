#include "ShareFunction.h"
#include "IDefine.h"
#include "tinyxml/tinyxml.h"
#include "tinyxml/md5.h"
#include "tinyxml/tinystr.h"


namespace share
{
    int LoadServerXML(const char* filename)
    {
        char fpath[MAX_FILENAME_LEN];
        memset(fpath, 0 ,MAX_FILENAME_LEN);
        sprintf(fpath, "%s%s", func::FileExePath, filename);

        //��ʼ��XML����
        if (func::__ServerInfo == nullptr)
        {
            func::__ServerInfo = new func::ConfigXML();
            memset(func::__ServerInfo, 0 , sizeof(func::ConfigXML));
        }

        //tinyxml����
        TiXmlDocument xml;
        if (!xml.LoadFile(fpath)) //���xml�ļ�����ʧ�ܵĻ�
        {
            LOG_MSG("load config_server.xml is error.. \n");
            return -1;
        }
        TiXmlElement* xmlRoot = xml.RootElement();//��ȡxml�ĸ��ڵ�
        if(xmlRoot == NULL)//���xml�ļ�Ϊ�ջ��߸�ʽ����ȷ�Ļ�
        {
            LOG_MSG("xmlRoot == NULL.. \n");
            return -1;
        }

        //�ҵ����ڵ�
        TiXmlElement* xmlNode = xmlRoot->FirstChildElement("server");

        memcpy(func::__ServerInfo->SafeCode,xmlNode->Attribute("SafeCode"),20);
        memcpy(func::__ServerInfo->Head,xmlNode->Attribute("Head"),2);

        func::__ServerInfo->Port = atoi(xmlNode->Attribute("Port"));
        func::__ServerInfo->ID = atoi(xmlNode->Attribute("ID"));
        func::__ServerInfo->Type = 0;
        func::__ServerInfo->MaxPlayer = atoi(xmlNode->Attribute("MaxUser"));
        func::__ServerInfo->MaxConnect = atoi(xmlNode->Attribute("MaxConnect"));
        func::__ServerInfo->MaxAccept = atoi(xmlNode->Attribute("MaxAccept"));
        func::__ServerInfo->MaxRece = atoi(xmlNode->Attribute("MaxRece"));
        func::__ServerInfo->MaxSend = atoi(xmlNode->Attribute("MaxSend"));
        func::__ServerInfo->RCode = atoi(xmlNode->Attribute("CCode"));
        func::__ServerInfo->Version = atoi(xmlNode->Attribute("Version"));
        func::__ServerInfo->ReceOne = atoi(xmlNode->Attribute("ReceOne")) *1024;//��kΪ��λ
        func::__ServerInfo->ReceMax = atoi(xmlNode->Attribute("ReceMax")) *1024;
        func::__ServerInfo->SendOne = atoi(xmlNode->Attribute("SendOne")) *1024;
        func::__ServerInfo->SendMax = atoi(xmlNode->Attribute("SendMax")) *1024;
        func::__ServerInfo->HeartTime = atoi(xmlNode->Attribute("HeartTime"));
        return 0;
    }

    int LoadClientXML(const char* filename)
    {
          char fpath[MAX_FILENAME_LEN];
        memset(fpath, 0 ,MAX_FILENAME_LEN);
        sprintf(fpath, "%s%s", func::FileExePath, filename);

        //��ʼ��XML����
        if (func::__ClientInfo == nullptr)
        {
            func::__ClientInfo = new func::ConfigXML();
            memset(func::__ClientInfo, 0 , sizeof(func::ConfigXML));
        }

        //tinyxml����
        TiXmlDocument xml;
        if (!xml.LoadFile(fpath)) //���xml�ļ�����ʧ�ܵĻ�
        {
            LOG_MSG("load config_client.xml is error.. \n");
            return -1;
        }
        TiXmlElement* xmlRoot = xml.RootElement();//��ȡxml�ĸ��ڵ�
        if(xmlRoot == NULL)//���xml�ļ�Ϊ�ջ��߸�ʽ����ȷ�Ļ�
        {
            LOG_MSG("xmlRoot == NULL.. \n");
            return -1;
        }

        //��ȡclient�ӽڵ���Ϣ
        TiXmlElement* xmlNode = xmlRoot->FirstChildElement("client");

        memcpy(func::__ClientInfo->SafeCode,xmlNode->Attribute("SafeCode"),20);
        memcpy(func::__ClientInfo->Head,xmlNode->Attribute("Head"),2);
        
        func::__ClientInfo->MaxPlayer = atoi(xmlNode->Attribute("MaxUser"));
        func::__ClientInfo->MaxConnect = atoi(xmlNode->Attribute("MaxConnect"));
        func::__ClientInfo->RCode = atoi(xmlNode->Attribute("CCode"));
        func::__ClientInfo->Version = atoi(xmlNode->Attribute("Version"));
        func::__ClientInfo->ReceOne = atoi(xmlNode->Attribute("ReceOne")) *1024;//��kΪ��λ
        func::__ClientInfo->ReceMax = atoi(xmlNode->Attribute("ReceMax")) *1024;
        func::__ClientInfo->SendOne = atoi(xmlNode->Attribute("SendOne")) *1024;
        func::__ClientInfo->SendMax = atoi(xmlNode->Attribute("SendMax")) *1024;
        func::__ClientInfo->HeartTime = atoi(xmlNode->Attribute("HeartTime"));
        func::__ClientInfo->AutoTime = atoi(xmlNode->Attribute("AutoTime"));

        //��ȡserver�ӽڵ���Ϣ
        xmlNode = xmlRoot->FirstChildElement("server");
        int num = atoi(xmlNode->Attribute("num"));
        char str[10];//�ڵ�����
        for(int i=1; i<=num; i++)//client������
        {
            memset(&str, 0, 10);
            sprintf(str, "client%d", i);//client��
            xmlNode = xmlRoot->FirstChildElement(str);

            func::ServerListXML* serverList = new func::ServerListXML();
            memcpy(serverList->IP, xmlNode->Attribute("IP"), 16);
            serverList->Port = atoi(xmlNode->Attribute("Port"));
            serverList->ID = atoi((xmlNode->Attribute("ID")));

            func::__ServerListInfo.push_back(serverList);
        }
        
        return 0;
    }
    
    bool InitData()
    {
        //���ú���ָ��
        func::MD5str = &utils::EncryptMD5str;
        //��ʼ������
        func::InitData();
        //2.��ʼ��XML
        int errs = LoadServerXML("config_server.xml");
        if (errs < 0) return false;
        errs = LoadClientXML("config_client.xml");
        if (errs < 0) return false;

        LOG_MSG("serverxml:%s-%d-%d\n", func::__ServerInfo->Head,func::__ServerInfo->ID,func::__ServerInfo->Port);
        LOG_MSG("clientxml:%s-%d-%d\n", func::__ClientInfo->Head,func::__ClientInfo->Version,func::__ClientInfo->RCode);

        for(int i = 0; i < func::__ServerListInfo.size(); i++)
            LOG_MSG("cleintxml:%s-%d-%d\n",func::__ServerListInfo[i]->IP,func::__ServerListInfo[i]->Port,func::__ServerListInfo[i]->ID);
        return true;
    }
}


