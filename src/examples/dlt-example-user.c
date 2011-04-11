/*
 * Dlt Client console utilities - Diagnostic Log and Trace
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-example-user.c                                            **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */
#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#include "dlt.h"
#include "dlt_common.h" /* for dlt_get_version() */

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length);

DLT_DECLARE_CONTEXT(mycontext);

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version);

    printf("Usage: dlt-example-user [options] message\n");
    printf("Generate DLT messages and store them to file or send them to daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -d delay      Milliseconds to wait between sending messages (Default: 500)\n");
    printf("  -f filename   Use local log file instead of sending to daemon\n");
    printf("  -n count      Number of messages to be generated (Default: 10)\n");
    printf("  -g            Switch to non-verbose mode (Default: verbose mode)\n");
    printf("  -a            Enable local printing of DLT messages (Default: disabled)\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    int vflag = 0;
    int gflag = 0;
    int aflag = 0;
    char *dvalue = 0;
    char *fvalue = 0;
    char *nvalue = 0;
    char *message = 0;

    int index;
    int c;

	char *text;
	int num,maxnum;
	int delay;

    opterr = 0;

    while ((c = getopt (argc, argv, "vgad:f:n:")) != -1)
    {
        switch (c)
        {
        case 'v':
        {
            vflag = 1;
            break;
        }
        case 'g':
        {
            gflag = 1;
            break;
        }
        case 'a':
        {
            aflag = 1;
            break;
        }
        case 'd':
        {
            dvalue = optarg;
            break;
        }
        case 'f':
        {
            fvalue = optarg;
            break;
        }
        case 'n':
        {
            nvalue = optarg;
            break;
        }
        case '?':
        {
            if (optopt == 'd' || optopt == 'f' || optopt == 'n')
            {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if (isprint (optopt))
            {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            }
            else
            {
                fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
            }

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            abort ();
        }
        }
    }

    for (index = optind; index < argc; index++)
    {
        message = argv[index];
    }

    if (message == 0)
    {
        /* no message, show usage and terminate */
        fprintf(stderr,"ERROR: No message selected\n");
        usage();
        return -1;
    }

    if (fvalue)
    {
        /* DLT is intialised automatically, except another output target will be used */
        if (dlt_init_file(fvalue)<0) /* log to file */
        {
            return -1;
        }
    }

    DLT_REGISTER_APP("LOG","Test Application for Logging");
    DLT_REGISTER_CONTEXT(mycontext,"TEST","Test Context for Logging");

    DLT_REGISTER_INJECTION_CALLBACK(mycontext, 0xFFF, dlt_user_injection_callback);

    text = message;

    if (gflag)
    {
        DLT_NONVERBOSE_MODE();
    }

    if (aflag)
    {
        DLT_ENABLE_LOCAL_PRINT();
    }

    if (nvalue)
    {
        maxnum = atoi(nvalue);
    }
    else
    {
		maxnum = 10;
    }

    if (dvalue)
    {
        delay = atoi(dvalue) * 1000;
    }
    else
    {
        delay = 500 * 1000;
    }

    if (gflag)
    {
        /* DLT messages to test Fibex non-verbose description: dlt-example-non-verbose.xml */
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,10);
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,11,DLT_UINT16(1011));
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,12,DLT_UINT32(1012),DLT_UINT32(1013));
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,13,DLT_UINT8(123),DLT_FLOAT32(1.12));
        DLT_LOG_ID(mycontext,DLT_LOG_INFO,14,DLT_STRING("DEAD BEEF"));
    }

    for (num=0;num<maxnum;num++)
    {
        printf("Send %d %s\n",num,text);


        if (gflag)
        {
            /* Non-verbose mode */
            DLT_LOG_ID(mycontext,DLT_LOG_WARN,num,DLT_INT(num),DLT_STRING(text));
        }
        else
        {
            /* Verbose mode */
            DLT_LOG(mycontext,DLT_LOG_WARN,DLT_INT(num),DLT_STRING(text));
        }

        if (delay>0)
        {
            usleep(delay);
        }
    }

    sleep(1);

    DLT_UNREGISTER_CONTEXT(mycontext);

    DLT_UNREGISTER_APP();

    dlt_free();

    return 0;

}

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];

    printf("Injection %d, Length=%d \n",service_id,length);
    if (length>0)
    {
        dlt_print_mixed_string(text,1024,data,length,0);
        printf("%s \n", text);
    }

    return 0;
}

