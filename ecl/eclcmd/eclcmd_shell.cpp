/*##############################################################################

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################## */
#include <stdio.h>
#include "jlog.hpp"
#include "jfile.hpp"
#include "jargv.hpp"
#include "jprop.hpp"

#include "build-config.h"

#include "eclcmd.hpp"
#include "eclcmd_core.hpp"

#define INIFILE "ecl.ini"
#define SYSTEMCONFDIR CONFIG_DIR
#define DEFAULTINIFILE "ecl.ini"
#define SYSTEMCONFFILE ENV_CONF_FILE

//=========================================================================================

#ifdef _WIN32
#include "process.h"
#endif

int EclCMDShell::callExternal(ArgvIterator &iter)
{
    const char *argv[100];
    StringBuffer cmdstr("ecl-");
    cmdstr.append(cmd.sget());
    int i=0;
    argv[i++]=cmdstr.str();
    if (optHelp)
        argv[i++]="help";
    for (; !iter.done(); iter.next())
        argv[i++]=iter.query();
    argv[i]=NULL;
//TODO - add common routine or use existing in jlib
#ifdef _WIN32
    if (_spawnvp(_P_WAIT, cmdstr.str(), const_cast<char **>(argv))==-1)
#else
    if (execvp(cmdstr.str(), const_cast<char **>(argv))==-1)
#endif
    {
        switch(errno)
        {
        case ENOENT:
            fprintf(stderr, "ecl '%s' command not found\n", cmd.sget());
            return 1;
        default:
            fprintf(stderr, "ecl '%s' command error %d\n", cmd.sget(), errno);
            return 1;
        }
    }
    return 0;
}

int EclCMDShell::processCMD(ArgvIterator &iter)
{
    Owned<IEclCommand> c = factory(cmd.get());
    if (!c)
    {
        if (cmd.length())
        {
            if (runExternals)
                return callExternal(iter);
            fprintf(stderr, "ecl '%s' command not found\n", cmd.sget());
        }
        usage();
        return 1;
    }
    if (optHelp)
    {
        c->usage();
        return 0;
    }
    if (!c->parseCommandLineOptions(iter))
        return 0;

    if (!c->finalizeOptions(globals))
        return 0;

    return c->processCMD();
}

void EclCMDShell::finalizeOptions(IProperties *globals)
{
}

int EclCMDShell::run()
{
    try
    {
        if (!parseCommandLineOptions(args))
            return 1;

        if (!optIniFilename)
        {
            if (checkFileExists(INIFILE))
                optIniFilename.set(INIFILE);
            else
            {
                StringBuffer fn(SYSTEMCONFDIR);
                fn.append(PATHSEPSTR).append(DEFAULTINIFILE);
                if (checkFileExists(fn))
                    optIniFilename.set(fn);
            }
        }

        globals.setown(createProperties(optIniFilename, true));
        finalizeOptions(globals);

        return processCMD(args);
    }
    catch (IException *E)
    {
        StringBuffer m("Error: ");
        fputs(E->errorMessage(m).newline().str(), stderr);
        E->Release();
        return 2;
    }
#ifndef _DEBUG
    catch (...)
    {
        ERRLOG("Unexpected exception\n");
        return 4;
    }
#endif
    return 0;
}

//=========================================================================================

bool EclCMDShell::parseCommandLineOptions(ArgvIterator &iter)
{
    if (iter.done())
    {
        usage();
        return false;
    }

    bool boolValue;
    for (; !iter.done(); iter.next())
    {
        const char * arg = iter.query();
        if (iter.matchFlag(optHelp, "help"))
            continue;
        if (*arg!='-')
        {
            cmd.set(arg);
            iter.next();
            break;
        }
        if (iter.matchFlag(boolValue, "--version"))
        {
            fprintf(stdout, "%s\n", BUILD_TAG);
            return false;
        }
        StringAttr tempArg;
        if (iter.matchOption(tempArg, "-brk"))
        {
#if defined(_WIN32) && defined(_DEBUG)
            unsigned id = atoi(tempArg.sget());
            if (id == 0)
                DebugBreak();
            else
                _CrtSetBreakAlloc(id);
#endif
        }
    }
    return true;
}

//=========================================================================================

void EclCMDShell::usage()
{
    fprintf(stdout,"\nUsage:\n"
        "    ecl [--version] <command> [<args>]\n\n"
           "Commonly used commands:\n"
           "   deploy    create an HPCC workunit from a local archive or shared object\n"
           "   publish   add an HPCC workunit to a query set\n"
           "\nRun 'ecl help <command>' for more information on a specific command\n\n"
    );
}
