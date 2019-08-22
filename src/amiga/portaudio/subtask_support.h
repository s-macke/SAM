#ifndef __SUBTASK_SUPPORT_H__
    #define __SUBTASK_SUPPORT_H__

/**************************************************************/
/* Functions for easy and secure spawning/killing of subtasks */
/* taken from MUI subtask example - A.F. March 10th 2004      */
/**************************************************************/


#include <exec/exec.h>
    
struct SubTaskMsg
{
    struct Message stm_Message;
    WORD           stm_Command;
    APTR           stm_Parameter;
    LONG           stm_Result;
};

struct SubTask
{
    struct Task      *st_Task;    /* sub task pointer */
    struct MsgPort   *st_Port;    /* allocated by sub task */
    struct MsgPort   *st_Reply;   /* allocated by main task */
    APTR              st_Data;    /* more initial data to pass to the sub task */
    struct SubTaskMsg st_Message; /* Message buffer */
};


#define STC_START 0
#define STC_STOP  1
#define STC_STARTUP  -2
#define STC_SHUTDOWN -1



LONG SendSubTaskMsg(struct SubTask *st,WORD command,APTR params);
struct SubTask *SpawnSubTask(char *name,VOID (*func)(VOID),APTR data);
VOID KillSubTask(struct SubTask *st);

/* the following functions are called from the sub task */
VOID ExitSubTask(struct SubTask *st,struct SubTaskMsg *stm);
struct SubTask *InitSubTask(void);



#endif    
