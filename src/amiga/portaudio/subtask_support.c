/**************************************************************/
/* Functions for easy and secure spawning/killing of subtasks */
/* taken from MUI subtask example - A.F. March 10th 2004      */
/**************************************************************/

#include "subtask_support.h"
#include <proto/exec.h>
//#include <dos/dosextens.h>
#include <proto/dos.h>
#include <utility/tagitem.h>
#include <dos/dostags.h>
#include <sys/types.h>




LONG SendSubTaskMsg(struct SubTask *st,WORD command,APTR params)
{
    st->st_Message.stm_Message.mn_ReplyPort = st->st_Reply;
    st->st_Message.stm_Message.mn_Length    = sizeof(struct SubTaskMsg);
    st->st_Message.stm_Command              = command;
    st->st_Message.stm_Parameter            = params;
    st->st_Message.stm_Result               = 0;

    PutMsg(command==STC_STARTUP ? &((struct Process *)st->st_Task)->pr_MsgPort : st->st_Port,(struct Message *)&st->st_Message);
    WaitPort(st->st_Reply);
    GetMsg(st->st_Reply);

    return(st->st_Message.stm_Result);
}

struct SubTask *SpawnSubTask(char *name,VOID (*func)(VOID),APTR data)
{
    struct SubTask *st;

    st = (struct SubTask*) AllocVec(sizeof(struct SubTask),MEMF_PUBLIC|MEMF_CLEAR);
    if (st)
    {
    	st->st_Reply = CreateMsgPort();
        if (st->st_Reply)
        {
            st->st_Data = data;

            st->st_Task = (struct Task *)CreateNewProcTags(NP_Entry,(unsigned long)func,NP_Name,(unsigned long)name,TAG_DONE);
            if (st->st_Task)
            {
                if (SendSubTaskMsg(st,STC_STARTUP,st))
                {
                    return(st);
                }
            }
            DeleteMsgPort(st->st_Reply);
        }
        FreeVec(st);
    }
    return(NULL);
}

VOID KillSubTask(struct SubTask *st)
{
    SendSubTaskMsg(st,STC_SHUTDOWN,st);
    DeleteMsgPort(st->st_Reply);
    FreeVec(st);
}

/* the following functions are called from the sub task */

VOID ExitSubTask(struct SubTask *st,struct SubTaskMsg *stm)
{
    /*
    ** We reply after a Forbid() to make sure we're really gone
    ** when the main task continues.
    */

    if (st->st_Port)
        DeleteMsgPort(st->st_Port);

    Forbid();
    stm->stm_Result = FALSE;
    ReplyMsg((struct Message *)stm);
}

struct SubTask *InitSubTask(void)
{
    struct Task *me = FindTask(NULL);
    struct SubTask *st;
    struct SubTaskMsg *stm;

    /*
    ** Wait for our startup message from the SpawnSubTask() function.
    */

    WaitPort(&((struct Process *)me)->pr_MsgPort);
    stm  = (struct SubTaskMsg *)GetMsg(&((struct Process *)me)->pr_MsgPort);
    st   = (struct SubTask *)stm->stm_Parameter;

    if ((st->st_Port = CreateMsgPort()))
    {
        /*
        ** Reply startup message, everything ok.
        ** Note that if the initialization fails, the code falls
        ** through and replies the startup message with a stm_Result
        ** of 0 after a Forbid(). This tells SpawnSubTask() that the
        ** sub task failed to run.
        */

        stm->stm_Result = TRUE;
        ReplyMsg((struct Message *)stm);
        return(st);
    }
    else
    {
        ExitSubTask(st,stm);
        return(NULL);
    }
}


