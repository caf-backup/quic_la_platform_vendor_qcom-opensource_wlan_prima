#ifndef PHYLOGMSG_H
#define PHYLOGMSG_H



//common header between firmware and application, something like logmsg.h
typedef enum
{
    WLANFW_PHY_LOG_MSG_ASICAGCSETGAINLUT_ERROR1_PARAMS,
    WLANFW_PHY_LOG_MSG_ASICAGCCALCGAINLUTSFORCHANNEL_PARAMS,
    WLANFW_PHY_LOG_MSG_ERROR_MININDEX_EXCEEDS_MAXINDEX,

    NUM_WLANFW_PHY_LOG_MSGS
}ePhyFwLogMsgs;


// log levels defined in: .../wlanfw/src/inc/config.h
typedef enum
{
    LOG_P = WLANFW_LOG_ALWAYS,
    LOG_E = WLANFW_LOG_ERROR,
    LOG_W = WLANFW_LOG_WARNING,
    LOG_1 = WLANFW_LOG_1,
    LOG_2 = WLANFW_LOG_2,
    LOG_3 = WLANFW_LOG_3,
    LOG_4 = WLANFW_LOG_4,
    
    NUM_WLANFW_PHY_LOG_MSG_PRIORITIES
}ePhyFwLogPriority;

#define MAX_LOG_MSG_ARGS 10

typedef struct 
{
    ePhyFwLogPriority logPriority;
    ePhyFwLogMsgs logId;
    unsigned int numArgs;   //each arg is 32 bits
}sPhyFwLogOutput;



#endif // PHYLOGMSG_H
