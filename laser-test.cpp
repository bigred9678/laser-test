#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

/*******************************************************************************************************
Objects:
    `<` directional laser light source sending light to the left.
    `>` directional laser light source sending light to the right.
    `/` mirror that bounces light 90 degrees (right → up, left → down, down → left, up → right).
    `\` mirror that bounces light 90 degrees (right → down, left → up, down → right, up → left).
    (Beware inputting `\` in languages that use this as a special escape character)
    `*` powered refractor that splrowIts light equally (duplicating the same power) in each cardinal direction (up, right, down, left) except for the incoming direction.
    `a` thru `z` : A lowercase letter that uniquely identifies a bulb. Bulbs are lrowIt when hrowIt by lasers, explained below.
    `.` Empty space (no object at this location).

Rules:
    1. Light hitting a laser emitter is blocked.
    2. Light hitting light does not cause any interference (light continues as normal).
        1. Light from different sources can co-exist in the same spot.
    3. Light hitting a bulb lights up the bulb and the light propagation is stopped.
        1. Bulbs may be hit by multiple lights, and the effects are additive (+1 per hit from *different* lights).
        2. Light split from the same source only adds +1 to a bulb’s power for each unique incoming direction, but is additive for each unique light source..
            1. For example, if one laser happened to stack 2 beams incoming from the left to a bulb (using refractors), this is still just +1 power (so effectively, stacking *the same light source* in one direction is meaningless).
            2. Another hit on that bulb from the left from a *different* laser is another +1 power.
            3. Another hit on that bulb from another direction from the same laser is another +1 power.
        3. Unlit bulbs are visualized as a lower-case letter.
        4. Lit bulbs are visualized as a capital letter.
        
Approach:
- allocate and ingest grid strings into array, store grid width (line break)
- save location of emitters
- save bulb data
- fire lasers from emitters, replace dots with laser, update bulb power
- print grid, print bulb power, cleanup

*********************************************************************************************************/
void traceLaserRecursive(char* laserPtr, int dir);

struct Emitter {
    char* ePtr = 0;
    int pos = 0; 
    int row = 0; 
    int dir = 0; 
    char type;
    int power = 0;

    friend std::ostream& operator<<(std::ostream& os, const Emitter& e) {
        return os << e.type << 
            " (" << "*: " << (void*)e.ePtr << " pos:" << e.pos << " row:" << e.row << " dir:" << e.dir << " pow: " << e.power << ")";
    }
};

// cardinal directions
enum Directions
{
    NORTH = 0,
    EAST,
    SOUTH,
    WEST
};

char* fullGrid = nullptr;
char* fullGridEnd = nullptr;
int gridSize = 0;
int gridRowWidth = 0;
vector<Emitter> laserEmitters = {};
map<char, Emitter> lightBulbs = {};     // maps are already sorted by key
vector<char*> splittersTouched = {};    // list of emitters touched by light source to prevent infinite recursion


// return row of given pointer
int getGridRow(char* posPtr)
{
    if (posPtr != nullptr && posPtr >= fullGrid && posPtr < fullGridEnd) 
        return (int)((posPtr - fullGrid) / gridRowWidth);

    return 0; // not right obviously, but just to save time and prevent crash
}

// save emitter found at position c 
void saveEmitter(char* c, int row)
{
    if (*c == '<' || *c == '>')
    {
        Emitter e;
        e.ePtr = c;
        e.pos = c - fullGrid;
        e.row = row;
        e.type = *c;
        e.dir = e.type == '<' ? WEST : EAST; // emitters are always EAST/WEST 
        laserEmitters.push_back(e);
        cout << "Saving Emitter " << e << endl;
    }
}

// save bulb found at position c
void saveBulb(char* c, int row)
{
    if (isalpha(*c))
    {
        // check for dupe
        if (lightBulbs.count((char)tolower(*c)) > 0)
        {
            cout << "dupe bulb found! do something" << endl;
            return;
        }

        Emitter b;
        b.ePtr = c;
        b.pos = c - fullGrid;
        b.row = row;
        b.type = (char)tolower(*c);
        b.power = 0;
        lightBulbs[b.type] = b;
        cout << "Saving Bulb " << b << endl;
    }
}

// store grid data into character array for easier pointer traversal. store any emitters or bulbs that are found
void ingestGrid(vector<string>& gridArray)
{
    int numRows = gridArray.size();
    char* gridWalkerPtr = 0;
    int row = 0;
    for (auto gridIt=gridArray.begin(); gridIt!=gridArray.end(); ++gridIt, ++row)
    {
        string gridRowStr = *gridIt;
        gridRowWidth = gridRowStr.size();

        if (fullGrid == 0)
        {
            gridSize = gridRowWidth * numRows;
            fullGrid = new char[gridSize+1];
            fullGridEnd = fullGrid + gridSize;
            gridWalkerPtr = fullGrid;
        }

        for (auto it=gridRowStr.begin(); it!=gridRowStr.end(); ++it)
        {
            *gridWalkerPtr = (*it);
            // cout << (void*)gridWalkerPtr << " " << *gridWalkerPtr << endl;
            if (*gridWalkerPtr == '<' || *gridWalkerPtr == '>')
            {
                saveEmitter(gridWalkerPtr, row);
            }
            else if (isalpha(*gridWalkerPtr))
            {
                saveBulb(gridWalkerPtr, row);
            }
            gridWalkerPtr++;
        }
    }

    cout << gridSize << " " << gridRowWidth << " " << fullGrid << endl;
}

// check if character is an emitter or a bulb
string emitterTypes = "/\\*"; // exclude guns (<>) so they don't fire when hit by other lasers
bool isEmitterOrBulb(const char& type)
{
    if (isalpha(type) || emitterTypes.find_first_of(type) != string::npos)
    {
        return true;
    }
    return false;
}

// check if character is a laser emitter gun (<>)
bool isLaserGun(const char& type)
{
    if (type == '<' || type == '>')
    {
        return true;
    }

    return false;
}

// when a laser comes in contact with a bulb, "light it" by changing to uppercase and incrementing power
void lightBulb(char* bulbPos)
{
    if (bulbPos != nullptr)
    {
        char baseType = (char)tolower(*bulbPos);
        if (lightBulbs.count(baseType) > 0)
        {
            *bulbPos = (char)toupper(*bulbPos);
            lightBulbs[baseType].type = *bulbPos;
            lightBulbs[baseType].power++;
        }
    }
}

// draw the laser character at the given position
// '|' for NORTH|SOUTH
// '-' for EAST|WEST
// '+' if a laser crosses another going in the perpendicular direction
void drawLaser(char* gridPtr, int dir)
{
    if (gridPtr != nullptr)
    {
        // place a '+' if laser cross
        if (dir == NORTH || dir == SOUTH)
        {
            if (*gridPtr == '-')
                *gridPtr = '+';
            else
                *gridPtr = '|';
        }
        else if (dir == EAST || dir == WEST) 
        {
            if (*gridPtr == '|')
                *gridPtr = '+';
            else
                *gridPtr = '-';
        }
    }
}

/**********************************************************************************************************
    MIRROR

    when a laser hits a mirror it reflects in a direction depending on the incoming laser direction and 
    the direction of the mirror

               ^              ^
               |              |
    /<--    -->/    -->\      \<--
    |                  |
    v                  v

    Parameters:
    char* laserPtr - position of laser on grid as a pointer
    int dir - incoming laser direction
    int row - current row of laser position on grid
**********************************************************************************************************/
void reflectLaser(char* laserPtr, int dir)
{
    if (laserPtr == nullptr)
    {
        cout << "ERROR: invalid laser point" << endl;
        return;
    }

    // mirrors reflect depending on the direction of the incoming laser       
    cout << "Mirror Hit! " << *laserPtr << " at " << (laserPtr - fullGrid) << " from " << dir << endl;

    // take incoming direction and reflect in alternate direction
    int reflectDir = NORTH; // init
    if (dir == NORTH) reflectDir = *laserPtr == '/' ? EAST : WEST;
    else if (dir == EAST) reflectDir = *laserPtr == '/' ? NORTH : SOUTH;
    else if (dir == SOUTH) reflectDir = *laserPtr == '/' ? WEST : EAST;
    else if (dir == WEST) reflectDir = *laserPtr == '/' ? SOUTH : NORTH;

    char* currLaserPtr = laserPtr;
    int laserRow = getGridRow(currLaserPtr);
    int currRow, rowDir, colDir = 0;

    // start position is next space away from mirror in reflection direction
    if (reflectDir == EAST || reflectDir == WEST) 
    {
        rowDir = reflectDir == EAST ? 1 : -1;
        colDir = 0;
        currLaserPtr = laserPtr + rowDir;
        currRow = laserRow;
    }
    else // NORTH|SOUTH
    {
        rowDir = 0;
        colDir = reflectDir == NORTH ? -1 : 1;
        currLaserPtr = laserPtr + (gridRowWidth * colDir);
        currRow = getGridRow(currLaserPtr);
    }
    // traverse laser and do not exceed beginning or end of grid
    while (currLaserPtr >= fullGrid && currLaserPtr < fullGridEnd)
    {
        cout << (void*)currLaserPtr << *currLaserPtr << " " << currRow << endl;
        // stop if laser travels off the side edges, indicated by row changes
        if ((reflectDir == EAST || reflectDir == WEST) &&
            currRow != laserRow) 
            break;
        
        if (isEmitterOrBulb(*currLaserPtr))
        {
            traceLaserRecursive(currLaserPtr, reflectDir);
            break;
        }

        if (isLaserGun(*currLaserPtr))
        {
            // stop at other guns
            cout << "Hit another gun! STOP!" << endl;
            break; // stop here so laser doesn't shoot through objects
        }
        
        drawLaser(currLaserPtr, reflectDir);

        if (reflectDir == EAST || reflectDir == WEST) 
        {
            currLaserPtr += rowDir;
        }
        else // NORTH|SOUTH
        {
            currLaserPtr += (gridRowWidth * colDir);
            currRow = getGridRow(currLaserPtr);
        }
    }
}

/**********************************************************************************************************
    SPLITTER

    when a laser hits a splitter, it emits a new laser in every cardinal direction except the direction of
    the incoming laser

            ^
            |
         <--*-->
            |
            v

    Parameters:
    char* laserPtr - position of laser on grid as a pointer
    int dir - incoming laser direction
    int row - current row of laser position on grid
**********************************************************************************************************/
void splitLaser(char* laserPtr, int dir)
{
    // splitters shoot in every direction except the direction of the incoming laser
    cout << "Splitter Hit!" << " at " << *laserPtr << " from " << dir << endl;

    if (std::find(splittersTouched.begin(), splittersTouched.end(), laserPtr) != splittersTouched.end())
    {
        cout << "Splitter already touched. Skip!" << endl;
        return;
    }

    splittersTouched.push_back(laserPtr);
    
    char* currLaserPtr = laserPtr;
    int laserRow = getGridRow(currLaserPtr);
    int outDir = dir; // init
    int currRow = laserRow; // init
    int colDir, rowDir = 0;

    // trace in every orthogonal cardinal direction, except the incoming one
    for (int currDir=NORTH; currDir<=WEST; ++currDir)
    {
        // init excluding emitter position, use orig position to start each direction
        if (currDir == NORTH) 
        {
            // ignore if incoming laser is from opposite direction
            if (dir==SOUTH)
                continue;

            cout << "Splitting NORTH from " << dir << endl;
            outDir = NORTH;
            currLaserPtr = laserPtr - gridRowWidth;
            currRow = laserRow - 1;
            rowDir = 0;
            colDir = -1;
        }
        else if (currDir == EAST)
        {
            if (dir==WEST)
                continue;

            cout << "Splitting EAST from " << dir << endl;
            outDir = EAST;
            currLaserPtr = laserPtr + 1;
            currRow = getGridRow(currLaserPtr);
            rowDir = 1;
            colDir = 0;
        }
        else if (currDir == SOUTH)
        {
            if (dir==NORTH)
                continue;

            cout << "Splitting SOUTH from " << dir << endl;
            outDir = SOUTH;
            currLaserPtr = laserPtr + gridRowWidth;
            currRow = laserRow + 1;
            rowDir = 0;
            colDir = 1;
        }
        else if (currDir == WEST)
        {
            if (dir==EAST)
                continue;

            cout << "Splitting WEST from " << dir << endl;
            outDir = WEST;
            currLaserPtr = laserPtr - 1;
            currRow = getGridRow(currLaserPtr);
            rowDir = -1;
            colDir = 0;
        }

        // traverse laser and do not exceed beginning or end of grid
        while (currLaserPtr >= fullGrid && currLaserPtr < fullGridEnd)
        {
            cout << (void*)currLaserPtr << " " << *currLaserPtr << " " << currRow << endl;
            if ((currDir == EAST || currDir == WEST) &&
                currRow != laserRow) // stop if laser travels off the side edges
                break;
            
            if (isEmitterOrBulb(*currLaserPtr))
            {
                cout << "emitter or bulb found at " << (int)(currLaserPtr - fullGrid) << endl;
                traceLaserRecursive(currLaserPtr, currDir);
                break;
            }

            if (isLaserGun(*currLaserPtr))
            {
                // stop at other guns
                cout << "Hit another gun! STOP!" << endl;
                break; // stop here so laser doesn't shoot through objects
            }
        
            drawLaser(currLaserPtr, currDir);

            if (colDir != 0) // NORTH|SOUTH
            {
                // traverse up or down
                currLaserPtr += (gridRowWidth * colDir);
                currRow += colDir;
            }
            else // EAST|WEST
            {
                // traverse left or right
                currLaserPtr += rowDir;
                currRow = getGridRow(currLaserPtr);
            }
        }
    }
}

/**********************************************************************************************************
    called when the laser hits an object 

    bulb - light bulb and stop
    mirror - reflects in one direction
    splitter - splits in cardinal directions except for incoming direction

    Parameters:
    char* laserPtr - position of laser on grid
    int dir - incoming direction of laser
    int row - current row of laser position on grid
**********************************************************************************************************/
void traceLaserRecursive(char* laserPtr, int dir)
{
    if (laserPtr == nullptr)
    {
        cout << "ERROR: invalid type" << endl;
        return;
    }

    // check for bulb
    if (isalpha(*laserPtr))
    {
        cout << "Light Bulb! " << *laserPtr << " at " << (int)(laserPtr - fullGrid) << endl;
        // this is a bulb, end of the road
        lightBulb(laserPtr);
        return;
    }
    // check for mirror
    else if (*laserPtr == '/' || *laserPtr == '\\')
    {
        reflectLaser(laserPtr, dir);
    }
    // check for splitter
    else if (*laserPtr == '*')
    {
        splitLaser(laserPtr, dir);
    }
}

// Lasers are emitted from a given emitter in EAST or WEST direction
// Lasers stop at edges or other emitters
//      Emitter& e - instance of emitter
void emitLaser(Emitter& e)
{
    if (e.ePtr == nullptr)
    {
        cout << "ERROR: invalid emitter" << endl;
        return;
    }

    // emitters always fire in the facing direction, one way, until it hits something or exits the edge
    cout << "Emit Laser! " << e.type << " at " << e.pos << endl;
    splittersTouched.clear(); // clear out store splitters from other emitters
    int currDir = e.type == '<' ? WEST : EAST;
    char* currPtr = e.ePtr + (currDir == WEST ? -1: 1); // skip the emitter itself
    int currRow = getGridRow(currPtr);
    while (currPtr >= fullGrid && currPtr < fullGridEnd && currRow == e.row) // don't exceed beginning/end or row
    {
        if (isEmitterOrBulb(*currPtr))
        {
            traceLaserRecursive(currPtr, currDir);
            break; // stop here so laser doesn't shoot through objects
        }

        if (isLaserGun(*currPtr))
        {
            // stop at other guns
            cout << "Hit another gun! STOP!" << endl;
            break;
        }
        
        drawLaser(currPtr, currDir);

        currPtr = currPtr + (currDir == WEST ? -1 : 1);
        currRow = getGridRow(currPtr);
    }
    cout << endl;
}

// iterate over all laser emitters found during ingestion
void fireLasers()
{
    cout << "Fire Lasers!" << endl;
    for (Emitter& e: laserEmitters)
    {
        cout << "Emitter : " << e << endl;
        emitLaser(e);
        cout << fullGrid << endl;
    }
}

void printGrid() 
{
    cout << "Grid" << endl;
    int row = 0;
    for (char* p=fullGrid; p < fullGridEnd; ++p)
    {
        int currRow = getGridRow(p);
        if (currRow > row)
        {
            cout << endl;
            row = currRow;
        }
        cout << *p;
    }
}

void printBulbs() 
{
    cout << "Bulbs" << endl;
    for (const auto& [type, b]: lightBulbs)
    {
        cout << "bulb: " << b.type << " power: " << b.power << endl;
    }
}

void reset()
{
    laserEmitters.clear();
    lightBulbs.clear();
    splittersTouched.clear();
    delete[] fullGrid;
    fullGrid = nullptr;
    fullGridEnd = nullptr;
}

int main() 
{
    vector<string> simple = {
        ".../.<",
        "<*...>",
        ">..a..",
    };

    vector<string> complex = {
        ".../..*..*.<...d",
        "................",
        "...\\.....a....<.",
        ".>....\\......b..",
        "...c............",
    };

    vector<string> test = {
        ">..*....\\..",
    "...a.<..h..",
    ".>.*..../.<",
    "...i...\\...",
    };

    ingestGrid(test);
    fireLasers();

    printGrid();
    cout << endl;
    printBulbs();

    reset();

    return 0;
}