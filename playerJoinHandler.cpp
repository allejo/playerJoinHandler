/*
Copyright (C) 2015 Vladimir "allejo" Jimenez

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
#include <vector>

#include "bzfsAPI.h"
#include "plugin_utils.h"

class PlayerJoinHandler : public bz_Plugin
{
public:
    virtual const char* Name () {return "Player Join Handler";}
    virtual void Init (const char* config);
    virtual void Event (bz_EventData *eventData);
    virtual void Cleanup (void);

    std::map<std::string, double> playerSessions;
};

BZ_PLUGIN(PlayerJoinHandler)

void PlayerJoinHandler::Init (const char* commandLine)
{
    Register(bz_eGetAutoTeamEvent);
    Register(bz_ePlayerPartEvent);

    if (!bz_BZDBItemExists("_rejoinTime"))
    {
        bz_setBZDBInt("_rejoinTime", 120);
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
        case bz_eGetAutoTeamEvent: // This event is called for each new player is added to a team
        {
            bz_GetAutoTeamEventData_V1* autoTeamData = (bz_GetAutoTeamEventData_V1*)eventData;

            std::unique_ptr<bz_BasePlayerRecord> pr(bz_getPlayerByIndex(autoTeamData->playerID));
            int rejoinTime = bz_getBZDBInt("_rejoinTime");
            std::string bzID = pr->bzID.c_str();

            if ((bz_isCountDownActive() || bz_isCountDownInProgress() || bz_isCountDownPaused()) && autoTeamData->team != eObservers)
            {
                if (!pr->verified)
                {
                    autoTeamData->handled = true;
                    autoTeamData->team = eObservers;

                    bz_sendTextMessage(BZ_SERVER, autoTeamData->playerID, "Your callsign is not verified, you will not be able to join any non-observer team during a match.");
                    bz_sendTextMessage(BZ_SERVER, autoTeamData->playerID, "Please register your callsign or ensure you are authenticating properly.");
                }
                else
                {
                    if ((playerSessions.find(bzID) == playerSessions.end()) || (playerSessions[bzID] + rejoinTime < bz_getCurrentTime()))
                    {
                        autoTeamData->handled = true;
                        autoTeamData->team = eObservers;

                        bz_sendTextMessage(BZ_SERVER, autoTeamData->playerID, "An active match is currently in progress. You have been automatically moved to the observer team to avoid disruption.");
                        bz_sendTextMessage(BZ_SERVER, autoTeamData->playerID, "If you intend to substitute another player, you may now rejoin as a player.");
                    }
                }
            }
        }
        break;

        case bz_ePlayerPartEvent: // This event is called each time a player leaves a game
        {
            bz_PlayerJoinPartEventData_V1* partData = (bz_PlayerJoinPartEventData_V1*)eventData;
            bz_BasePlayerRecord &pr = partData->record;

            if (pr->verified)
            {
                std::string bzID = pr->bzID.c_str();

                playerSessions[bzID] = bz_getCurrentTime();
            }
        }
        break;

        default: break;
    }
}
