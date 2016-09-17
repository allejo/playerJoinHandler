/*
    Copyright (C) 2016 Vladimir "allejo" Jimenez

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <map>
#include <memory>
#include <sstream>

#include "bzfsAPI.h"
#include "plugin_utils.h"

// Define plugin name
const std::string PLUGIN_NAME = "Player Join Handler";

// Define plugin version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 2;
const int BUILD = 16;

class PlayerJoinHandler : public bz_Plugin
{
public:
    virtual const char* Name ();
    virtual void Init (const char* config);
    virtual void Event (bz_EventData *eventData);
    virtual void Cleanup (void);

    typedef std::map<std::string, double> SessionList;
    virtual bool sessionExists(SessionList &list, std::string target);

    SessionList bzidSessions, ipSessions;
    std::string bzdb_SessionTime, bzdb_AllowUnregistered;
};

BZ_PLUGIN(PlayerJoinHandler)

const char* PlayerJoinHandler::Name (void)
{
    static std::string pluginBuild = "";

    if (!pluginBuild.size())
    {
        std::ostringstream pluginBuildStream;

        pluginBuildStream << PLUGIN_NAME << " " << MAJOR << "." << MINOR << "." << REV << " (" << BUILD << ")";
        pluginBuild = pluginBuildStream.str();
    }

    return pluginBuild.c_str();
}

void PlayerJoinHandler::Init (const char* /*commandLine*/)
{
    Register(bz_eGetAutoTeamEvent);
    Register(bz_ePlayerPartEvent);

    bzdb_SessionTime       = "_sessionTime";
    bzdb_AllowUnregistered = "_allowUnverified";

    if (!bz_BZDBItemExists(bzdb_SessionTime.c_str()))
    {
        bz_setBZDBInt(bzdb_SessionTime.c_str(), 120);
    }

    if (!bz_BZDBItemExists(bzdb_AllowUnregistered.c_str()))
    {
        bz_setBZDBBool(bzdb_AllowUnregistered.c_str(), false);
    }
}

void PlayerJoinHandler::Cleanup (void)
{
    Flush();
}

void PlayerJoinHandler::Event (bz_EventData *eventData)
{
    switch (eventData->eventType)
    {
        case bz_eGetAutoTeamEvent:
        {
            bz_GetAutoTeamEventData_V1* autoTeamData = (bz_GetAutoTeamEventData_V1*)eventData;

            std::unique_ptr<bz_BasePlayerRecord> pr(bz_getPlayerByIndex(autoTeamData->playerID));

            std::string bzID       = pr->bzID.c_str();
            std::string ipAddress  = pr->ipAddress.c_str();
            bool allowUnregistered = bz_getBZDBBool(bzdb_AllowUnregistered.c_str());

            if ((bz_isCountDownActive() || bz_isCountDownInProgress() || bz_isCountDownPaused()) && autoTeamData->team != eObservers)
            {
                if ((pr->verified  && !sessionExists(bzidSessions, bzID))    ||
                    (!pr->verified && !sessionExists(ipSessions, ipAddress)) ||
                    (!pr->verified && !allowUnregistered))
                {
                    autoTeamData->handled = true;
                    autoTeamData->team = eObservers;

                    bz_sendTextMessage(BZ_SERVER, autoTeamData->playerID, "An active match is currently in progress. You have been automatically moved to the observer team to avoid disruption.");

                    if (pr->verified)
                    {
                        bz_sendTextMessage(BZ_SERVER, autoTeamData->playerID, "If you intend to substitute another player, you may now rejoin as a player.");
                    }
                    else
                    {
                        if (!allowUnregistered)
                        {
                            bz_sendTextMessage(BZ_SERVER, autoTeamData->playerID, "This server only allows registered players to join as a subtitute, please use a registered account.");
                        }
                    }
                }
            }
        }
        break;

        case bz_ePlayerPartEvent:
        {
            bz_PlayerJoinPartEventData_V1* partData = (bz_PlayerJoinPartEventData_V1*)eventData;
            bz_BasePlayerRecord* &pr = partData->record;

            std::string ipAddress = pr->ipAddress.c_str();
            std::string bzID = pr->bzID.c_str();

            if (pr->verified)
            {
                bzidSessions[bzID] = bz_getCurrentTime();
            }
            else
            {
                ipSessions[ipAddress] = bz_getCurrentTime();
            }
        }
        break;

        default: break;
    }
}

bool PlayerJoinHandler::sessionExists(SessionList &list, std::string target)
{
    int rejoinTime = bz_getBZDBInt(bzdb_SessionTime.c_str());

    return (list.count(target)) && (list[target] + rejoinTime > bz_getCurrentTime());
}
