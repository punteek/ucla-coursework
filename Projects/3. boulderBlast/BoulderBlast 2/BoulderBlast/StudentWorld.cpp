#include "StudentWorld.h"
#include "Level.h"
#include "Actor.h"
#include <vector>
#include <string>

using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

int StudentWorld::init()
{
    int status = loadLevel();
    // if level data exists but isn't in the proper format
    if (status == -1) {
        return GWSTATUS_LEVEL_ERROR;
    }
    
    //
    //        // if no level data or last level completed was lvl 99
    //        if (1) {
    //            return GWSTATUS_PLAYER_WON;
    //        }
    
    
    
    
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::updateDisplay() {
    
    
    
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp
int StudentWorld::move()
{
    // Update the Game Status Line
    updateDisplay(); // update the score/lives/level text at screen top
    // The term "actors" refers to all robots, the Player, Goodies,
    // Boulders, Jewels, Holes, Bullets, the Exit, etc.
    // Give each actor a chance to do something
    
    for (auto actor : m_actors)
    {
        if (m_player->isAlive()) {
            m_player->doSomething();
        }
        if (actor->isAlive())
        {
            // ask each actor to do something (e.g. move)
            actor->doSomething();
            if (! m_player->isAlive())
                return GWSTATUS_PLAYER_DIED;
//            if (thePlayerCompletedTheCurrentLevel())
//            {
//                increaseScoreAppropriately();
//                return GWSTATUS_FINISHED_LEVEL;
//            }
        }
    }
//    // Remove newly-dead actors after each tick
//    removeDeadGameObjects(); // delete dead game objects
//    // Reduce the current bonus for the Level by one
//    reduceLevelBonusByOne();
//    // If the player has collected all of the Jewels on the level, then we
//    // must expose the Exit so the player can advance to the next level
//    if (thePlayerHasCollectedAllOfTheJewelsOnTheLevel())
//        exposeTheExitInTheMaze(); // make the exit Active
//    // return the proper result
//    if (! m_player->isAlive())
//        return GWSTATUS_PLAYER_DIED;
//    if (thePlayerCompletedTheCurrentLevel())
//    {
//        increaseScoreAppropriately();
//        return GWSTATUS_FINISHED_LEVEL;
//    }
    // the player hasn’t completed the current level and hasn’t died, so
    // continue playing the current level
    return GWSTATUS_CONTINUE_GAME;
    return 0;
}

int StudentWorld::loadLevel() {
    
    std::string	curLevel = "level00.dat";
    Level lev(assetDirectory());
    
    // format level
    int level = getLevel();
    std::string formattedLevel;
    if (level % 10 > 0) {
        formattedLevel = "level" + std::to_string(level) + ".dat";
    }
    else {
        formattedLevel = "level0" + std::to_string(level) + ".dat";
    }
    
    Level::LoadResult result = lev.loadLevel(formattedLevel);
    if (result	==	Level::load_fail_file_not_found	||
        result	==	Level:: load_fail_bad_format)
        return -1; //	something	bad	happened!
    
    for (int x = 0; x < VIEW_WIDTH; x++) {
        for (int y = 0; y < VIEW_HEIGHT; y++) {
            Level::MazeEntry item =	lev.getContentsOf(x,y);
            switch (item) {
                case Level::wall: {
                    m_actors.push_back(new Wall(this, x, y));
                    break;
                }
                case Level::player:
                    m_player = new Player(this, x, y);
                    break;
                default:
                    break;
            }
        }
    }
    
    return 1;
}

// free all actors when player completes current level or dies
void StudentWorld::cleanUp() {
    delete m_player;
    
    std::vector<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); ) {
        std::vector<Actor*>::iterator it2 = it;
        it = m_actors.erase(it);
        delete *it2;
    }
    
}