#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>//file size
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

char * pLocalPort = "69";

/*

int sOk = 1;
int dbug = 0;
int dbugOk = 0;
int dDbug = 0;
int dDbugOk = 0;
*/
int sOk = 1;
int dbug = 0;
int dbugOk = 0;
int dDbug = 0;
int dDbugOk = 0;

void say (char * string)
{
    printf("%s\n",string);
}
void sayOk (void * string)
{
    if(sOk)
    {
        say (string);
    }
}
void debug (void * string)
{
    if(dbug)
    {
        say (string);
    }
}
void debugOk (void * string)
{
    if(dbugOk)
    {
        say (string);
    }
}
void dDebug (void * string)
{
    if(dDbug)
    {
        say (string);
    }
}
void dDebugOk (void * string)
{
    if(dDbugOk)
    {
        say (string);
    }
}

char * pDefaultLocalFilename = "rfc867_sr.txt";
char * pDefaultOpenReadType = "rb";
//char   cDefaultReadRequest = 'r';
//char * pDefaultMode = "octet";
char * pDefaultOpenWriteType = "wb";

struct session
{
    char cClientOrServer;
    struct addrinfo getaddrinfoSourceLinkList;
    struct addrinfo * pGetaddrinfoResultLinkList;
    //struct addrinfo * pFirstValidGetaddrinfoResultLinkListSend;
    //struct addrinfo * pFirstValidGetaddrinfoResultLinkListReceive;
    int localSocket;
    //char * pRemoteHost;//name or ip
    //char * pRemoteFilename;
    char * pLocalFilename;
    char cReadOrWrite;
    char * pMode;
    int requsetOk;
    struct sockaddr_storage remoteAddressInformation;
    socklen_t sizeRemoteAddressInformation;
    char aIpRemote[INET6_ADDRSTRLEN];
    const char * pTextIpRemote;

    clock_t commandTimer;
    clock_t fileTimer;
};
struct session * pSession;

///read and write file
#define maxReadOnce 512
struct aboutReadFile
{
    char * pFilename;
    char * pOpenReadType;
    char aReadBuffer[maxReadOnce];
    int sizeReadUnit;
    int sizeReadData;
    FILE * pReadFile;
    int sizeFile;
};
struct aboutReadFile * pAboutReadFile;

struct aboutWriteFile
{
    char * pFilename;
    char * pOpenWriteType;
    char * pWriteBuffer;
    int sizeWriteUnit;
    int sizeWriteData;
    FILE * pWriteFile;
    int sizeFile;
};
struct aboutWriteFile * pAboutWriteFile;
///read and write file end

///pack and unpack
#define udpLoadReceiveMax 516
#define withoutOpcodeMax 514 //blockNr + 512
#define blockDataMax 512
    ///send
struct prepareTftpErrorPacket
{
	unsigned short expectedOpcode;
	unsigned short lastOpcode;
	int opcodeOk;
	unsigned short expectedBlockNr;
	unsigned short lastBlockNr;
	int blockNrOk;
	int opcodeAndBlockNrOk;
};
struct prepareTftpErrorPacket * pPrepareTftpErrorPacket;
struct prepareTftpAckPacket
{
    struct prepareTftpAckPacket * pOriginalAddress;
	unsigned short opcode;
	unsigned short blockNr;
};
struct prepareTftpAckPacket * pPrepareTftpAckPacket;
struct prepareTftpRequestPacket
{
	unsigned short opcode;
    char * pFilename;
    int sizeFilename;
    char * pMode;
    int sizeMode;
    int sizeWithoutOpcode;
//    int sizeTftpRequestPacket;
};
struct prepareTftpRequestPacket * pPrepareTftpRequestPacket;
struct prepareTftpDataPacket
{
    struct prepareTftpDataPacket * pOriginalAddress;
    int sizeBlockDataToSend;
	unsigned short opcode;
	unsigned short blockNrTftpDataPacket;
    int reachEofSend;
};
struct prepareTftpDataPacket * pPrepareTftpDataPacket;
struct prepareUdpLoad
{
    void * pUdpLoadToSend;
    int sizeUdpLoadToSend;
    int sizeUdpLoadHasSend;//for debug
};
struct prepareUdpLoad * pPrepareUdpLoad;
    ///send end

struct tftpPacket
{
	unsigned short opcodeNetOrder;
	char aWithoutOpcode[withoutOpcodeMax];
};
struct tftpPacket * pTftpPacket;
struct tftpRequestPacket
{
	unsigned short opcodeNetOrder;
    char aWithoutOpcode[withoutOpcodeMax];
};
struct tftpRequestPacket * pTftpRequestPacket;
struct tftpDataPacket
{
	unsigned short opcodeNetOrder;
	unsigned short blockNrNetOrder;
	char aBlockData[blockDataMax];
};
struct tftpDataPacket * pTftpDataPacket;
struct tftpAckPacket
{
	unsigned short opcodeNetOrder;
	unsigned short blockNrNetOrder;
};
struct tftpAckPacket * pTftpAckPacket;

    ///receive
struct handleUdpLoad
{
    char aUdpLoadReceiveBuffer[udpLoadReceiveMax];//void * pUdpLoadHasReceive;
    int sizeUdpLoadHasReceive;
};
struct handleUdpLoad * pHandleUdpLoad;
struct handleTftpPacket
{
    int sizeWithoutOpcode;
    int sizeBlockDataHasReceive;
	unsigned short opcode;
	unsigned short blockNrTftpDataPacket;
};
struct handleTftpPacket * pHandleTftpPacket;
struct handleTftpRequestPacket
{
	unsigned short opcode;
    char * pFilename;
    int sizeFilename;
    char * pMode;
    int sizeMode;
};
struct handleTftpRequestPacket * pHandleTftpRequestPacket;
struct handleTftpDataPacket
{
	unsigned short opcode;
    int blockNrTftpDataPacket;
    char * pBlockDataReceive;
    int sizeBlockDataReceive;
    int reachEofReceive;
};
struct handleTftpDataPacket * pHandleTftpDataPacket;
struct handleTftpAckPacket
{
	unsigned short opcode;
	unsigned short blockNr;
};
struct handleTftpAckPacket * pHandleTftpAckPacket;
    ///receive end
///pack and unpack end

/// declaration of functions
int sendTheRequestFile();
int receiveTheRequestFile();
/// declaration of functions end

char aTextTimeStamp[] = "2010-00-00_00:00:00.000_0";
char * timeStamp()
{
    struct timeb tp;
    struct tm ltime;
    ftime(&tp);
    localtime_r(&tp.time,&ltime);
    sprintf(aTextTimeStamp,
            "%04d-%02d-%02d_%02d:%02d:%02d.%03d_%d",
            ltime.tm_year+1900,
            ltime.tm_mon+1,
            ltime.tm_mday,
            ltime.tm_hour,
            ltime.tm_min,
            ltime.tm_sec,
            tp.millitm,
            ltime.tm_wday//What day is it.
            );
    aTextTimeStamp[23] = '\0';//don't need What day is it.
    return aTextTimeStamp;
/*
    char * stringTimeStamp;
    time_t rawtime;
    time ( &rawtime );
    stringTimeStamp = ctime( (time_t*) &rawtime);
    stringTimeStamp[24] = '\0';
    return stringTimeStamp;
*/
}
int openReadFile()
{
    struct stat tStat;
    if(dbug)
    {
        printf("ok: openReadFile().start.\nok: file to open:%s.\n",pAboutReadFile->pFilename);
    }
    if ( (pAboutReadFile->pReadFile = fopen(pAboutReadFile->pFilename, pAboutReadFile->pOpenReadType)) == NULL)
    {
        say("ERR: openReadFile(): fopen().");
        exit(1);
    }
    else
    {
        debug("ok: openReadFile().");
    }
    stat(pAboutReadFile->pFilename, &tStat);
    pAboutReadFile->sizeFile = tStat.st_size;

    return 0;
}
int closeReadFile()
{
    if( fclose(pAboutReadFile->pReadFile) == 0)
    {
        debug("ok: closeReadFile().");
    }
    else
    {
        say("ERR: closeReadFile().");
        return 1;
    }
    return 0;
}
int openWriteFile()
{
    if(dbug)
    {
        printf("ok: openWriteFile().start.\nok: file to open:%s.\n",pAboutReadFile->pFilename);
    }
    if ( (pAboutWriteFile->pWriteFile = fopen(pAboutWriteFile->pFilename, pAboutWriteFile->pOpenWriteType)) == NULL)
    {
        say("ERR: fopen writeFile.");
        return 1;
    }

    return 0;
}
int closeWriteFile()
{
    fclose(pAboutWriteFile->pWriteFile);
    return 0;
}
int writeBinaryDataToFile()
{
    debug("ok: in writeBinaryDataToFile().");
    fwrite(pAboutWriteFile->pWriteBuffer, pAboutWriteFile->sizeWriteUnit, pAboutWriteFile->sizeWriteData, pAboutWriteFile->pWriteFile);

    say("ok: writeBinaryDataToFile().");
    return 0;
}
void * getBinaryIp(struct sockaddr * sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int checkOpcodeAndBlockNr()
{
    if(pPrepareTftpErrorPacket->lastOpcode == pPrepareTftpErrorPacket->expectedOpcode)
    {
        if(dbug)printf("ok: got expected opcode:%u.\n",pPrepareTftpErrorPacket->lastOpcode);
        pPrepareTftpErrorPacket->opcodeOk = 1;

        if(pPrepareTftpErrorPacket->lastBlockNr == pPrepareTftpErrorPacket->expectedBlockNr)
        {
            if(dbug)printf("ok: got expected blockNr:%u.\n",pPrepareTftpErrorPacket->lastBlockNr);
            pPrepareTftpErrorPacket->blockNrOk = 1;
            pPrepareTftpErrorPacket->expectedBlockNr++;
        }
        else
        {
            printf("ERR: blockNr. got:%u, but expected:%u.\n",pPrepareTftpErrorPacket->lastBlockNr,pPrepareTftpErrorPacket->expectedBlockNr);
            pPrepareTftpErrorPacket->blockNrOk = 0;
        }
    }
    else
    {
        printf("ERR: opcode. got:%u, but expected:%u.\n",pPrepareTftpErrorPacket->lastOpcode,pPrepareTftpErrorPacket->expectedOpcode);
        pPrepareTftpErrorPacket->opcodeOk = 0;
    }

    pPrepareTftpErrorPacket->opcodeAndBlockNrOk = pPrepareTftpErrorPacket->opcodeOk && pPrepareTftpErrorPacket->blockNrOk;
    if(pPrepareTftpErrorPacket->opcodeAndBlockNrOk)
    {
        debug("ok: checkOpcodeAndBlockNr().");
        return 0;
    }
    else
    {
        return 1;
    }
}

int initialize()
{
    debug("ok: in initialize().");
    pSession = malloc(sizeof(struct session));
    memset(pSession, '\0', sizeof(struct session) );
    pSession->cClientOrServer = 's';

    pAboutWriteFile = malloc(sizeof(struct aboutWriteFile));
    pAboutReadFile = malloc(sizeof(struct aboutReadFile));

    ///send
    pPrepareTftpErrorPacket = malloc(sizeof(struct prepareTftpErrorPacket));
    pPrepareTftpAckPacket = malloc(sizeof(struct prepareTftpAckPacket));
    pPrepareTftpDataPacket = malloc(sizeof(struct prepareTftpDataPacket));
    pPrepareUdpLoad = malloc(sizeof(struct prepareUdpLoad));
    //pPrepareTftpRequestPacket = malloc(sizeof(struct prepareTftpRequestPacket));//no need in server
    //pPrepareTftpPacket = malloc(sizeof(struct prepareTftpPacket));//no need in both hosts
    if(dbugOk){printf("ok: malloc: sizeof(struct tftpDataPacket):%d.\n",sizeof(struct tftpDataPacket));}

    pPrepareTftpDataPacket->pOriginalAddress = pTftpDataPacket = malloc(sizeof(struct tftpDataPacket));
    pPrepareTftpAckPacket->pOriginalAddress = pTftpAckPacket = malloc(sizeof(struct tftpAckPacket));
    ///send end

    ///receive
    //pTftpPacket = malloc(sizeof(struct tftpPacket));
    //pTftpRequestPacket = malloc(sizeof(struct tftpRequestPacket));

    pHandleUdpLoad = malloc(sizeof(struct handleUdpLoad));
    pHandleTftpPacket = malloc(sizeof(struct handleTftpPacket));
    pHandleTftpDataPacket = malloc(sizeof(struct handleTftpDataPacket));
    pHandleTftpAckPacket = malloc(sizeof(struct handleTftpAckPacket));
    pHandleTftpRequestPacket = malloc(sizeof(struct handleTftpRequestPacket));
    if(dbugOk){printf("ok: malloc: sizeof(struct tftpPacket):%d.\n",sizeof(struct tftpPacket));}
    ///receive end

    debug("ok: initialize()");
    return 0;
}

int getaddrinfoLocal()
{
    int getaddrinfoStatus;

    memset(&pSession->getaddrinfoSourceLinkList, 0, sizeof pSession->getaddrinfoSourceLinkList);
    pSession->getaddrinfoSourceLinkList.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    pSession->getaddrinfoSourceLinkList.ai_socktype = SOCK_DGRAM;
    pSession->getaddrinfoSourceLinkList.ai_flags = AI_PASSIVE; // use my IP

    if ((getaddrinfoStatus = getaddrinfo(NULL,//cause use localloop ip
                                        pLocalPort,
                                        &pSession->getaddrinfoSourceLinkList,//said use localloop ip
                                        &pSession->pGetaddrinfoResultLinkList
                                        )
        ) != 0)
    {
        fprintf(stderr, "ERR: getaddrinfo: %s\n", gai_strerror(getaddrinfoStatus));
        return 1;
    }

    debug("ok: getaddrinfoLocal().end.");
    return 0;
}
int tryCreateAndBind()
{
    struct addrinfo *pTempAddrinfo;
    for(pTempAddrinfo = pSession->pGetaddrinfoResultLinkList; pTempAddrinfo != NULL; pTempAddrinfo = pTempAddrinfo->ai_next)
    {
        if ((pSession->localSocket = socket(pTempAddrinfo ->ai_family,
                                            pTempAddrinfo ->ai_socktype,
                                            pTempAddrinfo ->ai_protocol
                                            )
            ) == -1
           )
        {
            perror("ERR: socket()");
            continue;
        }
        debug("ok: succeed create socket.");

        if (bind(pSession->localSocket,
                pTempAddrinfo->ai_addr,
                pTempAddrinfo->ai_addrlen
                ) == -1
            )
        {
            close(pSession->localSocket);
            perror("ERR: bind()");
            continue;
        }
        debug("ok: succeed bind socket.");
        break;
    }

    if (pTempAddrinfo == NULL)
    {
        fprintf(stderr, "server: failed to bind socket, p == NULL\n");
        return 2;
    }

    debug("ok: succeed create and bind.");
    return 0;
}







int packTftpDataPacket()
{
    int iFread = 0;
    char readTempChar;
    pTftpDataPacket = pPrepareTftpDataPacket->pOriginalAddress;

    pAboutReadFile->sizeReadUnit = sizeof(char);
    while( fread(&readTempChar,pAboutReadFile->sizeReadUnit,1, pAboutReadFile->pReadFile), !feof(pAboutReadFile->pReadFile) && !ferror(pAboutReadFile->pReadFile) )
    {
        if(dDbugOk){printf("ok: read %c\n", readTempChar);}
        pAboutReadFile->aReadBuffer[iFread] = readTempChar;
        if(dDbugOk){printf("ok: iFread and aReadBuffer[iFread]:%d,%c.\n",iFread,pAboutReadFile->aReadBuffer[iFread]);}
        iFread ++;
        if(iFread == maxReadOnce)
        {
            break;
        }
    }
    if (dDbugOk){printf("ok: while is ok, iFread:%d.\n",iFread);}
    if (feof(pAboutReadFile->pReadFile))
    {
        pPrepareTftpDataPacket->reachEofSend = 1;
        printf("ok: eof.\n");
        if(dbug){printf("ok: in feof(), iFread(last read):%dbytes.\n",iFread);}
//            aReadBuffer[iFread] = '\0';
    }
    if (ferror(pAboutReadFile->pReadFile))
    {
        printf("ERR read file.\n");
    }
    pAboutReadFile->sizeReadData = iFread;

    if (dDbugOk){printf("ok: aReadBuffer:%s.\n",pAboutReadFile->aReadBuffer);}
    if (dDbugOk){printf("ok: aReadBuffer[0]:%s.\n", &(pAboutReadFile->aReadBuffer[0]) );}
    if (dDbugOk){printf("ok: aReadBuffer[1]:%s.\n", &(pAboutReadFile->aReadBuffer[1]) );}

///pack data_packet and re-direct
    pPrepareTftpDataPacket->opcode = 3;//is data packet
    pPrepareTftpDataPacket->sizeBlockDataToSend = pAboutReadFile->sizeReadData;
    pTftpDataPacket->opcodeNetOrder = htons(pPrepareTftpDataPacket->opcode);
    pTftpDataPacket->blockNrNetOrder = htons(pPrepareTftpDataPacket->blockNrTftpDataPacket);
    memmove(pTftpDataPacket->aBlockData, pAboutReadFile->aReadBuffer, pPrepareTftpDataPacket->sizeBlockDataToSend);
    if(dbug){printf("ok: in prepareSend(), blockNrTftpDataPacket:%u.\n",pPrepareTftpDataPacket->blockNrTftpDataPacket);}
    if(dbug){printf("ok: in prepareSend(), sizeBlockDataToSend:%u.\n",pPrepareTftpDataPacket->sizeBlockDataToSend);}

    pPrepareUdpLoad->pUdpLoadToSend = (void *)pTftpDataPacket;
    pPrepareUdpLoad->sizeUdpLoadToSend = 4 + pPrepareTftpDataPacket->sizeBlockDataToSend;
///pack data_packet and re-direct end

    debug("ok: packTftpDataPacket().");
    return 0;
}
int packTftpAckPacket()
{
    debug("ok: packTftpAckPacket().start.");
    pTftpAckPacket = pPrepareTftpAckPacket->pOriginalAddress;
    pPrepareTftpAckPacket->opcode = 4;
    pTftpAckPacket->opcodeNetOrder = htons(pPrepareTftpAckPacket->opcode);
    pTftpAckPacket->blockNrNetOrder = htons(pPrepareTftpAckPacket->blockNr);

    pPrepareUdpLoad->pUdpLoadToSend = pTftpAckPacket;
    pPrepareUdpLoad->sizeUdpLoadToSend = sizeof(struct tftpAckPacket);

    if(dbug){printf("ok: ack blockNr:%d.",pPrepareTftpAckPacket->blockNr);}
    debug("ok: packTftpAckPacket().end.");
    return 0;
}
int packAndSendUdpPacket()
{
    if ((pPrepareUdpLoad->sizeUdpLoadHasSend = sendto(pSession->localSocket,
                                                   pPrepareUdpLoad->pUdpLoadToSend,
                                                   pPrepareUdpLoad->sizeUdpLoadToSend,
                                                   0,
                                                   (struct sockaddr *)&pSession->remoteAddressInformation,
                                                   sizeof (struct sockaddr)
                                                  )
     ) == -1
        )
    {
        perror("ERR: sendto");
        exit(1);
    }

    if(dbug)
    {
        printf("ok: sizeUdpLoadToSend: %dbytes.\n",pPrepareUdpLoad->sizeUdpLoadToSend);
        pSession->pTextIpRemote = inet_ntop(pSession->remoteAddressInformation.ss_family,
                                            getBinaryIp((struct sockaddr *)&pSession->remoteAddressInformation),
                                            pSession->aIpRemote,
                                            sizeof pSession->aIpRemote
                                            );
        printf("ok: sent to %s| ",pSession->pTextIpRemote);

    }
    if(dDbugOk){printf("%dbytes, %s.\n",pPrepareUdpLoad->sizeUdpLoadHasSend,(char*)pPrepareUdpLoad->pUdpLoadToSend);}
    else{if(dbug){printf("%dbytes.\n",pPrepareUdpLoad->sizeUdpLoadHasSend);}}

    printf("ok: sendData().%s.\n",timeStamp());
    return 0;
}

int tftpDataPacketSendOk()
{
    if(dbug){printf("ok: in tftpDataPacketSendOk(), blockNrTftpDataPacket:%u.\n",pPrepareTftpDataPacket->blockNrTftpDataPacket);}
    pPrepareTftpDataPacket->blockNrTftpDataPacket ++;
    return 0;
}

int receiveAndUnpackUdpPacket()
{
    pSession->sizeRemoteAddressInformation = sizeof pSession->remoteAddressInformation;
    if ((pHandleUdpLoad->sizeUdpLoadHasReceive = recvfrom(pSession->localSocket,
                                                        pHandleUdpLoad->aUdpLoadReceiveBuffer,
                                                        udpLoadReceiveMax,//dataReceiveMax-1,
                                                        0,
                                                        (struct sockaddr *)&pSession->remoteAddressInformation,
                                                        &pSession->sizeRemoteAddressInformation
                                                        )
         ) == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    pSession->pTextIpRemote = inet_ntop(pSession->remoteAddressInformation.ss_family,
                                        getBinaryIp((struct sockaddr *)&pSession->remoteAddressInformation),
                                        pSession->aIpRemote,
                                        sizeof pSession->aIpRemote
                                        );
    printf("ok: got packet from %s\n",pSession->pTextIpRemote);
    if(dDbugOk){printf("ok: recv: %dbytes, %s.\n", pHandleUdpLoad->sizeUdpLoadHasReceive,pHandleUdpLoad->aUdpLoadReceiveBuffer);}
    else{if(dbug)printf("ok: sizeUdpLoadHasReceive:%d.\n",pHandleUdpLoad->sizeUdpLoadHasReceive);}

    printf("ok: receiveAndUnpackUdpPacket().%s.\n",timeStamp());
    return 0;
}
int unpackTftpRequestPacket()
{
    debug("ok: unpackTftpRequestPacket().start.");
    pTftpRequestPacket = (struct tftpRequestPacket *)pTftpPacket;
    pHandleTftpRequestPacket->opcode = pHandleTftpPacket->opcode;
    pHandleTftpRequestPacket->pFilename = pTftpRequestPacket->aWithoutOpcode;
    pHandleTftpRequestPacket->sizeFilename = strlen(pHandleTftpRequestPacket->pFilename);
    pHandleTftpRequestPacket->pMode = (char*)(pHandleTftpRequestPacket->pFilename) + pHandleTftpRequestPacket->sizeFilename + 1;
    pHandleTftpRequestPacket->sizeMode = strlen(pHandleTftpRequestPacket->pMode);

    pSession->commandTimer = clock();

    pSession->pLocalFilename = pHandleTftpRequestPacket->pFilename;
    pSession->pMode = pHandleTftpRequestPacket->pMode;
    if(strcmp(pSession->pMode, "octet"))
    {
        say("ERR: octet type only.");
        exit(1);
    }

    if(pHandleTftpPacket->opcode == 1)//read request
    {
        pSession->cReadOrWrite = 'r';

        pPrepareTftpErrorPacket->expectedBlockNr = 1;

        sendTheRequestFile();
    }
    else//write request
    {
        pSession->cReadOrWrite = 'w';

        pPrepareTftpAckPacket->blockNr = 0;
        packTftpAckPacket();
        packAndSendUdpPacket();

        pPrepareTftpErrorPacket->expectedBlockNr = 1;

        receiveTheRequestFile();
    }
    pSession->commandTimer = clock() - pSession->commandTimer;
    printf("ok: handeling this command costs: %i s.\n",pSession->commandTimer/CLOCKS_PER_SEC);
    debug("ok: unpackTftpRequestPacket().end.");
    return 0;
}
int unpackTftpDataPacket()
{
    debug("ok: packet is data_packet.");
    pTftpDataPacket = (struct tftpDataPacket *) pTftpPacket;
    pHandleTftpDataPacket->opcode = pHandleTftpPacket->opcode;
    pHandleTftpDataPacket->blockNrTftpDataPacket = ntohs(pTftpDataPacket->blockNrNetOrder);
    pPrepareTftpErrorPacket->lastBlockNr = pHandleTftpDataPacket->blockNrTftpDataPacket;
    pHandleTftpDataPacket->sizeBlockDataReceive = pHandleTftpPacket->sizeWithoutOpcode - 2;
    pHandleTftpDataPacket->pBlockDataReceive = pTftpDataPacket->aBlockData;
    pAboutWriteFile->sizeFile += pHandleTftpDataPacket->sizeBlockDataReceive;
    if(pHandleTftpDataPacket->sizeBlockDataReceive < blockDataMax)
    {
        debug("ok: set: reachEofReceive = 1");
        pHandleTftpDataPacket->reachEofReceive = 1;
    }
    if(dbug){printf("ok: pHandleTftpDataPacket->blockNrTftpDataPacket:%d.\n",pHandleTftpDataPacket->blockNrTftpDataPacket);}

    pPrepareTftpErrorPacket->expectedOpcode = 3;
    //pPrepareTftpErrorPacket->expectedBlockNr = 1;
    if(checkOpcodeAndBlockNr())
    {
        debug("ERR: opcode or blockNr.");
        exit(1);
    }

    pPrepareTftpAckPacket->blockNr = pHandleTftpDataPacket->blockNrTftpDataPacket;
    packTftpAckPacket();
    packAndSendUdpPacket();

    pAboutWriteFile->pWriteBuffer = pHandleTftpDataPacket->pBlockDataReceive;
    pAboutWriteFile->sizeWriteData = pHandleTftpDataPacket->sizeBlockDataReceive;
    pAboutWriteFile->sizeWriteUnit = sizeof(char);
    if(dbug){printf("ok: sizeWriteData = sizeBlockDataReceive:%d.\n",pAboutWriteFile->sizeWriteData);}
    writeBinaryDataToFile();

    return 0;
}
int unpackTftpAckPacket()
{
    pTftpAckPacket = (struct tftpAckPacket *) pTftpPacket;
    pHandleTftpAckPacket->opcode = pHandleTftpPacket->opcode;
    pHandleTftpAckPacket->blockNr = ntohs(pTftpAckPacket->blockNrNetOrder);
    pPrepareTftpErrorPacket->lastBlockNr = pHandleTftpAckPacket->blockNr;

    if(dbug)printf("ok: unpackTftpAckPacket().end.blockNr:%u.\n",pHandleTftpAckPacket->blockNr);
    return 0;
}
int unpackTftpPacket()
{
    debug("ok: unpackTftpPacket().start.");
    pTftpPacket = (struct tftpPacket*) pHandleUdpLoad->aUdpLoadReceiveBuffer;

    ///unpack and re-direct //unpackTftpPacket(); return opcode //code transferred from client
    pHandleTftpPacket->sizeWithoutOpcode = pHandleUdpLoad->sizeUdpLoadHasReceive - 2;
    pHandleTftpPacket->opcode = ntohs(pTftpPacket->opcodeNetOrder);
    pPrepareTftpErrorPacket->lastOpcode = pHandleTftpPacket->opcode;
    if(dbug){printf("ok: unpackTftpPacket opcode:%d.\n",pHandleTftpPacket->opcode);}

    switch(pHandleTftpPacket->opcode)
    {
        case 1:
            debug("ok: packet type: read request.");
            unpackTftpRequestPacket();
            break;
        case 2:
            debug("ok: packet type: write request.");
            unpackTftpRequestPacket();
            break;
        case 3:
            debug("ok: packet type: data.");///code transferred from client
            if(pSession->requsetOk)//would be set in unpackTftpWriteRequestPacket()
            {
                unpackTftpDataPacket();
            }
            else
            {
                say("ERR: pSession->requsetOk is not confirmed");
                return 1;
            }
            break;
        case 4:
            unpackTftpAckPacket();
            break;
        default:
            say("ERR: unpackTftpPacket(): wrong packet type.");
            return 1;
    }


    //etc.
    ///unpack and re-direct end

    debug("ok: unpackTftpPacket().end.");

    return 0;
}

int sendTheRequestFile()//rrq //code from client
{
    pAboutReadFile->pFilename = pSession->pLocalFilename;
    pAboutReadFile->pOpenReadType = pDefaultOpenReadType;
    openReadFile();

    pPrepareTftpDataPacket->blockNrTftpDataPacket = 1;
    pPrepareTftpDataPacket->reachEofSend = 0;
    pSession->fileTimer = clock();

    while(1) //transfer all data of the file
    {
        debug("\nok: while: transfer all data of the file.start.");

        if(pPrepareTftpDataPacket->blockNrTftpDataPacket != 1)
        {
            ///receive ack
            receiveAndUnpackUdpPacket();
            unpackTftpPacket();

            pPrepareTftpErrorPacket->expectedOpcode = 4;
            //pPrepareTftpErrorPacket->expectedBlockNr = 1;
            if(checkOpcodeAndBlockNr())
            {
                debug("ERR: opcode or blockNr.");
                exit(1);
            }
        }

        ///when blockNr of ack packet ok
        packTftpDataPacket();
        packAndSendUdpPacket();

//            sleep(1);
        tftpDataPacketSendOk();

        if(pPrepareTftpDataPacket->reachEofSend)
        {
            debug("ok: sendTheRequestFile() reach eof.");
            break;
        }

        say("");
    }

    //receive ack //final ack
    receiveAndUnpackUdpPacket();
    unpackTftpPacket();

    pPrepareTftpErrorPacket->expectedOpcode = 4;
    //pPrepareTftpErrorPacket->expectedBlockNr = 1;
    if(checkOpcodeAndBlockNr())
    {
        debug("ERR: opcode or blockNr. when receiving final ack.");
        exit(1);
    }
    else
    {
        debug("ok: got final ack of final sent data.");
    }

    closeReadFile();
    pPrepareTftpDataPacket->blockNrTftpDataPacket = 1;
    pPrepareTftpDataPacket->reachEofSend = 0;

    pSession->fileTimer = clock() - pSession->fileTimer;
    pSession->fileTimer = pSession->fileTimer/CLOCKS_PER_SEC;
    if(pSession->fileTimer == 0)
    {
        pSession->fileTimer = 1;
    }
    printf("ok: send file speed: %i bytes/s.\n",pAboutReadFile->sizeFile/(pSession->fileTimer) );
    debug("ok: sendTheRequestFile()");
    return 0;
}

int receiveTheRequestFile()//wrq
{
    debug("ok: receiveTheRequestFile().start.");
    if(dbug)
    {
        printf("ok: filename:%s.\n",pSession->pLocalFilename);
        printf("ok: mode:%s.\n",pSession->pMode);
    }
    pAboutWriteFile->pFilename = pSession->pLocalFilename;
    pAboutWriteFile->pOpenWriteType = pDefaultOpenWriteType;
    openWriteFile();

    pSession->requsetOk = 1;
    pHandleTftpDataPacket->reachEofReceive = 0;
    pAboutWriteFile->sizeFile = 0;
    pSession->fileTimer = clock();

    while(1) ///the "while" of: receive all the data of remote file and write to local file
    {
        debug("\nok: \"while\": receive and write.start.");
        receiveAndUnpackUdpPacket();

        unpackTftpPacket();

        if(pHandleTftpDataPacket->reachEofReceive)
        {
            debug("\nok: reachEofReceive the \"while\" of: receive.");
            break;
        }
    }
    closeWriteFile();

    pSession->requsetOk = 0;
    pHandleTftpDataPacket->reachEofReceive = 0;

    pSession->fileTimer = clock() - pSession->fileTimer;
    pSession->fileTimer = pSession->fileTimer/CLOCKS_PER_SEC;
    if(pSession->fileTimer == 0)
    {
        pSession->fileTimer = 1;
    }
    printf("ok: receive file speed: %i bytes/s.\n",pAboutWriteFile->sizeFile/pSession->fileTimer );
    pAboutWriteFile->sizeFile = 0;


    debug("ok: openWriteFileAndReceiveAndSaveDataAndClose().end.");
    return 0;
}

int main(void)
{
    debug("ok: program start.");
    initialize();

    getaddrinfoLocal();

    tryCreateAndBind();
    freeaddrinfo(pSession->pGetaddrinfoResultLinkList);

    while(1) //each loop for each request
    {
        printf("\nok: waiting request ...\n\n");

        receiveAndUnpackUdpPacket();

        unpackTftpPacket();

        printf("\nok: a request end.%s.\n",timeStamp());
    }

    return 0;
}
