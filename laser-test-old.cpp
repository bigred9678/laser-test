#include <iostream>
#include <vector>
#include <map>
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
*********************************************************************************************************/
struct Emitter {
    int pos; 
    int row; 
    int dir; 
    char type;

    friend std::ostream& operator<<(std::ostream& os, const Emitter& e) {
        return os << e.type << " (" << "pos:" << e.pos << " row:" << e.row << " dir:" << e.dir << ")";
    }
};

struct Bulb {
    int pos;
    int row;
    char name;
    int power = 0;

    friend std::ostream& operator<<(std::ostream& os, const Bulb& b) {
        return os << b.name << " (" << "pos:" << b.pos << " row:" << b.row << " power:" << b.power << ")";
    }
};

vector<Emitter> emitters = {};
vector<Bulb> bulbs = {};

// Find any emitter matching the given character set. each emitter has a direction. the splitter (*) goes in every direction
// Store position and direction for later
void findEmitters(string& in, int row) 
{
    cout << "Find Emitters " << in << endl;
    string emitterChars = "<>/\\*";
    size_t charPos = in.find_first_of(emitterChars);
    while (charPos != string::npos)
    {
        Emitter emitter;
        emitter.pos = charPos;
        emitter.row = row;
        emitter.type = in[charPos];
        if (emitter.type == '>' || emitter.type == '\\')
            emitter.dir = 1;
        else if (emitter.type == '<' || emitter.type == '/')
            emitter.dir = -1;
        else
            emitter.dir = 0;
        cout << "Emitter Found: " << emitter << endl;
        emitters.push_back(emitter);

        charPos = in.find_first_of(emitterChars, charPos+1);
    }
}

// Find any bulb (alpha characters only). Save position and state for later
void findBulbs(string& in, int row)
{
    cout << "Find Bulbs " << in << endl;
    for (size_t i=0; i<in.size(); ++i)
    {
        if (isalpha(in[i]))
        {
            Bulb bulb;
            bulb.name = in[i];
            bulb.pos = i;
            bulb.row = row;
            bulb.power = 0;
            cout << "Bulb Found: " << bulb << endl;
            bulbs.push_back(bulb);
        }
    }
}

// New algorithm: saves one iteration by checking for everything at once
// Find All entities in the given row. bulbs are always alpha characters. emitters are from a specific set
// Save position and other state data for later
void findAllEntities(string& in, int row) 
{
    cout << "Find All Entities " << in << endl;
    string emitterChars = "<>/\\*";
    size_t pos = in.find_first_not_of('.');
    while (pos != string::npos)
    {
        if (isalpha(in[pos]))
        {
            Bulb bulb;
            bulb.name = in[pos];
            bulb.pos = pos;
            bulb.row = row;
            bulb.power = 0;
            cout << "Bulb Found: " << bulb << endl;
            bulbs.push_back(bulb);
        }
        else 
        {
            Emitter emitter;
            emitter.pos = pos;
            emitter.row = row;
            emitter.type = in[pos];
            if (emitter.type == '>' || emitter.type == '\\')
                emitter.dir = 1;
            else if (emitter.type == '<' || emitter.type == '/')
                emitter.dir = -1;
            else
                emitter.dir = 0;
            cout << "Emitter Found: " << emitter << endl;
            emitters.push_back(emitter);
        }

        pos = in.find_first_not_of('.', pos+1);
    }
}

void findCharacters(const vector<string>& in) 
{
    cout << "Find Characters" << endl;
    for (auto rowIt=in.begin(); rowIt != in.end(); ++rowIt)
    {
        string rowStr = *rowIt;
        cout << rowStr << endl;
        int row = rowIt - in.begin();

        findAllEntities(rowStr, row);
        // findEmitters(rowStr, row);
        // findBulbs(rowStr, row);
        cout << endl;
    }
    cout << endl;
}

// "fire" lasers from "<>" characters. 
// recursively branch from each emitter in its respective direction
void fireLasers(vector<string>& grid)
{
    cout << "Fire Lasers! There are " << emitters.size() << " emitters" << endl;
    // find collisions between emitters
    for (size_t i=0; i<emitters.size(); ++i)
    {
        Emitter start = emitters[i];
        char type = start.type;
        cout << "emitting from " << type << " at ("<< start.row << "," << start.pos << ") " << start.pos << endl;
        if (type == '<' || type == '>')
        {
            for (size_t j=0; j<emitters.size(); ++j)
            {
                Emitter end = emitters[j];
                string& gridRowStr = grid[start.row];
                cout << "checking " << gridRowStr << endl;
                if (start.row == end.row)
                {
                    // replace all characters in between on the row in the right direction
                    int dist = end.pos - start.pos;
                    bool laserHits = (dist * start.dir) > 0;
                    if (laserHits) // laser hits if dist is positive
                    {
                        int firstPos = start.pos < end.pos ? start.pos + 1 : end.pos + 1;
                        int lastPos = start.pos < end.pos ? end.pos : start.pos;
                        cout << "firstPos: " << firstPos << " lastPos: " << lastPos << " dist: " << dist << endl;
                        for (int dot=0; dot<lastPos-firstPos; ++dot)
                        {
                            int replacePos = firstPos + dot;
                            cout << "replacing " << replacePos << endl;
                            gridRowStr[replacePos] = '-';
                        }
                        cout << "Laser Hits: " << grid[start.row] << endl;
                    }
                }
            }
        }
        else if (type == '/' || type == '\\')
        {
            for (size_t j=0; j<emitters.size(); ++j)
            {
                Emitter end = emitters[j];
                if (start.pos == end.pos)
                {
                    // replace all characters in between on the column
                }
            }
        }
        else if (type == '*')
        {
            for (size_t j=0; j<emitters.size(); ++j)
            {
                Emitter end = emitters[j];
                if (start.row == end.row)
                {
                    // replace all characters in between on the row
                }
                else if (start.pos == end.pos)
                {
                    // replace all characters in between on the column
                }
            }
        }
    }
}


// ingest into vector, store grid width (line break)
// save location of emitters
// save bulb data
// emit lasers recursively, replace dots with laser, update bulb power
// print grid, print bulb power
string fullGrid = "";
int gridWidth = 0;
string emitterTypes = "/\\*"; // exclude guns so they don't fire when hit by other lasers
vector<Emitter> laserEmitters = {};
map<char, Bulb> lightBulbs = {};

// cardinal directions
enum Directions
{
    NORTH = 0,
    EAST,
    SOUTH,
    WEST
};

void saveEmitter(char& type, int idx)
{
    Emitter e;
    e.pos = idx;
    e.row = (int)(idx / gridWidth);
    e.type = type;
    e.dir = type == '<' ? WEST : EAST;
    laserEmitters.push_back(e);
    cout << "Saving Emitter " << e << endl;
}

void saveBulb(char& name, int idx)
{
    if (lightBulbs.count(name) > 0)
    {
        cout << "dupe bulb found! do something" << endl;
        return;
    }

    Bulb b;
    b.pos = idx;
    b.row = (int)(idx / gridWidth);
    b.name = name;
    b.power = 0;
    lightBulbs[name] = b;
    cout << "Saving Bulb " << b << endl;
}

void ingestGridIntoVector(vector<string>& gridArray)
{
    for (auto gridIt=gridArray.begin(); gridIt!=gridArray.end(); ++gridIt)
    {
        string gridRowStr = *gridIt;
        gridWidth = gridRowStr.size();
        for (auto& gridChar: *gridIt)
        {
            fullGrid += gridChar;
            if (gridChar == '<' || gridChar == '>')
            {
                saveEmitter(gridChar, fullGrid.size()-1);
            }
            else if (isalpha(gridChar))
            {
                saveBulb(gridChar, fullGrid.size()-1);
            }
        }
    }

    cout << gridWidth << fullGrid << endl;
}

bool isEmitterorBulb(char& type)
{
    if (isalpha(type) || emitterTypes.find_first_of(type) != string::npos)
    {
        return true;
    }
    return false;
}

bool isLaserGun(char& type)
{
    if (type == '<' || type == '>')
    {
        return true;
    }

    return false;
}

void lightBulb(char& type)
{
    char baseType = (char)tolower(type);
    if (lightBulbs.count(baseType) > 0)
    {
        lightBulbs[baseType].name = (char)toupper(type);
        lightBulbs[baseType].power++;
    }
}

void drawLaser(int pos, int dir)
{
    // place a '+' if laser cross
    if (fullGrid[pos] == '|' || fullGrid[pos] == '-')
    {
        fullGrid[pos] = '+';
    }
    else if (dir == NORTH || dir == SOUTH)
    {
        fullGrid[pos] = '|';
    }
    else if (dir == EAST || dir == WEST) 
    {
        fullGrid[pos] = '-';
    }
}

void emitLaserRecursive(char& type, int pos, int row, int dir)
{
    if (isalpha(type))
    {
        cout << "Light Bulb! " << type << " at " << pos << endl;
        // this is a bulb, end of the road
        fullGrid[pos] = (char)toupper(type);
        lightBulb(type);
        return;
    }
    else if (type == '<' || type == '>')
    {
        // emitters always fire in the facing direction, one way, until it hits something or exits the edge
        cout << "Fire Laser! " << type << " at " << pos << endl;
        int currDir = type == '<' ? WEST : EAST;
        int currPos=pos + (currDir == WEST ? -1 : 1); // skip the emitter itself
        int currRow = (int)(currPos / gridWidth);
        cout << "laserTouched: ";
        while (currPos >= 0 && currPos < fullGrid.size() && currRow == row)
        {
            cout << currPos << " ";

            char currChar = fullGrid.at(currPos);
            if (isEmitterorBulb(currChar))
            {
                emitLaserRecursive(currChar, currPos, row, currDir);
                break; // stop here so laser doesn't shoot through objects
            }

            if (isLaserGun(currChar))
            {
                // stop at other guns
                cout << "Hit another gun! STOP!" << endl;
                break;
            }
            
            drawLaser(currPos, currDir);

            currPos = currPos + (currDir == WEST ? -1 : 1);
            currRow = (int)(currPos / gridWidth);
        }
        cout << endl;
    }
    else if (type == '/' || type == '\\')
    {
        // mirrors reflect depending on the direction of the incoming laser       
        cout << "Mirror Hit! " << type << " at " << pos << " from " << dir << endl;

        int currDir = NORTH; // init
        if (dir == NORTH) currDir = type == '/' ? EAST : WEST;
        else if (dir == EAST) currDir = type == '/' ? NORTH : SOUTH;
        else if (dir == SOUTH) currDir = type == '/' ? WEST : EAST;
        else if (dir == WEST) currDir = type == '/' ? SOUTH : NORTH;

        int currPos, currRow, rowDir, colDir = 0;

        if (currDir == EAST || currDir == WEST) 
        {
            rowDir = currDir == EAST ? 1 : -1;
            colDir = 0;
            currPos = pos + rowDir;
            currRow = row;
        }
        else // NORTH|SOUTH
        {
            rowDir = 0;
            colDir = currDir == NORTH ? -1 : 1;
            currPos = pos + (gridWidth * colDir);
            currRow = (int)(currPos / gridWidth);
        }
        cout << "laserTouched: ";
        while (currPos >= 0 && currPos < fullGrid.size())
        {
            cout << currPos << " " << currRow << endl;
            if ((currDir == EAST || currDir == WEST) &&
                currRow != row) // stop if laser travels off the side edges
                break;
            
            char currChar = fullGrid.at(currPos);
            if (isEmitterorBulb(currChar))
            {
                emitLaserRecursive(currChar, currPos, currRow, currDir);
                break;
            }

            if (isLaserGun(currChar))
            {
                // stop at other guns
                cout << "Hit another gun! STOP!" << endl;
                break; // stop here so laser doesn't shoot through objects
            }
            
            drawLaser(currPos, currDir);

            if (currDir == EAST || currDir == WEST) 
            {
                currPos = currPos + rowDir;
            }
            else // NORTH|SOUTH
            {
                currPos = currPos + (gridWidth * colDir);
                currRow = (int)(currPos / gridWidth);
            }
        }
    }
    else if (type == '*')
    {
        // splitters shoot in every direction except the direction of the incoming laser
        cout << "Splitter Hit!" << " at " << pos << " from " << dir << endl;
        int currPos = pos; // init
        int outDir = dir; // init
        int currRow = row; // init
        int colDir, rowDir = 0;
        for (int currDir=NORTH; currDir<=WEST; ++currDir)
        {
            // init excluding emitter position, use orig position to start each direction
            if (currDir == NORTH && dir != SOUTH) // ignore if incoming laser is from opposite direction
            {
                cout << "Splitting NORTH from " << dir << endl;
                outDir = NORTH;
                currPos = pos - gridWidth;
                currRow = row - 1;
                rowDir = 0;
                colDir = -1;
            }
            else if (currDir == EAST && dir != WEST)
            {
                cout << "Splitting EAST from " << dir << endl;
                outDir = EAST;
                currPos = pos + 1;
                currRow = (int)(currPos / gridWidth);
                rowDir = 1;
                colDir = 0;
            }
            else if (currDir == SOUTH && dir != NORTH)
            {
                cout << "Splitting SOUTH from " << dir << endl;
                outDir = SOUTH;
                currPos = pos + gridWidth;
                currRow = row + 1;
                rowDir = 0;
                colDir = 1;
            }
            else if (currDir == WEST && dir != EAST)
            {
                cout << "Splitting WEST from " << dir << endl;
                outDir = WEST;
                currPos = pos - 1;
                currRow = (int)(currPos / gridWidth);
                rowDir = -1;
                colDir = 0;
            }

            while (currPos >= 0 && currPos<fullGrid.size())
            {
                cout << currPos << " " << currRow << endl;
                if ((currDir == EAST || currDir == WEST) &&
                    currRow != row) // stop if laser travels off the side edges
                    break;
                
                char currChar = fullGrid.at(currPos);
                if (isEmitterorBulb(currChar))
                {
                    emitLaserRecursive(currChar, currPos, currRow, currDir);
                    break;
                }

                if (isLaserGun(currChar))
                {
                    // stop at other guns
                    cout << "Hit another gun! STOP!" << endl;
                    break; // stop here so laser doesn't shoot through objects
                }
            
                drawLaser(currPos, currDir);

                if (colDir != 0) // NORTH|SOUTH
                {
                    // increment up or down
                    currPos += (gridWidth * colDir);
                    currRow += colDir;
                }
                else // EAST|WEST
                {
                    // increment left or right
                    currPos += rowDir;
                    currRow = (int)(currPos / gridWidth);
                }
            }
        }
    }
}

void fireLasers()
{
    cout << "Fire Lasers!" << endl;
    for (Emitter& e: laserEmitters)
    {
        cout << "Emitter : " << e << endl;
        emitLaserRecursive(e.type, e.pos, e.row, 0);
        cout << fullGrid << endl;
    }
}

void printGrid() 
{
    cout << "Grid" << endl;
    int row = 0;
    for (int pos=0; pos<fullGrid.size(); ++pos)
    {
        int currRow = (int)(pos / gridWidth);
        if (currRow > row)
        {
            cout << endl;
            row = currRow;
        }
        cout << fullGrid[pos];
    }
}

void printBulbs() 
{
    cout << "Bulbs" << endl;
    for (const auto& [type, b]: lightBulbs)
    {
        cout << "bulb: " << b.name << " power: " << b.power << endl;
    }
}

void reset()
{
    fullGrid.clear();
    laserEmitters.clear();
    lightBulbs.clear();
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

    // findCharacters(simple);
    // fireLasers(simple);

    // findCharacters(complex);
    // fireLasers(complex);

    ingestGridIntoVector(complex);
    fireLasers();

    printGrid();
    cout << endl;
    printBulbs();

    return 0;
}