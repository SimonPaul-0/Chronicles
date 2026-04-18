/*
 * ╔══════════════════════════════════════════════════════════════════════╗
 * ║          CHRONICLES OF THE ABYSS  ─  A Pure C++17 Terminal RPG      ║
 * ╠══════════════════════════════════════════════════════════════════════╣
 * ║  C++ Concepts Demonstrated:                                          ║
 * ║  ✦ Abstract Base Classes & Pure Virtual Functions                   ║
 * ║  ✦ Inheritance Hierarchies & Runtime Polymorphism                   ║
 * ║  ✦ Smart Pointers (unique_ptr, shared_ptr, dynamic_pointer_cast)    ║
 * ║  ✦ STL Containers (vector, map, unordered_map, tuple, pair)         ║
 * ║  ✦ Function Templates & Template Specialisation                     ║
 * ║  ✦ Lambda Functions & std::function (Observer Pattern)              ║
 * ║  ✦ File I/O with Custom Serialisation (Save / Load System)         ║
 * ║  ✦ Custom Exception Hierarchy                                       ║
 * ║  ✦ Operator Overloading (Stats +, +=, ==)                          ║
 * ║  ✦ Move Semantics & Perfect Forwarding                              ║
 * ║  ✦ STL Algorithms (any_of, find_if, transform, min, max, sort)      ║
 * ║  ✦ Scoped Enum Classes                                              ║
 * ║  ✦ Factory, Observer & Strategy Design Patterns                     ║
 * ║  ✦ Mersenne Twister RNG (mt19937)                                   ║
 * ╚══════════════════════════════════════════════════════════════════════╝
 */
 
#ifdef _WIN32
  #include <windows.h>
  #define CLEAR_CMD "cls"
#else
  #define CLEAR_CMD "clear"
#endif
 
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <functional>
#include <stdexcept>
#include <iomanip>
#include <cmath>
#include <limits>
#include <tuple>
#include <chrono>
#include <thread>
 
using namespace std;
 
// ═══════════════════════════════════════════════════════════════
//  RANDOM ENGINE  (Mersenne Twister seeded from hardware clock)
// ═══════════════════════════════════════════════════════════════
static mt19937 rng(static_cast<unsigned>(
    chrono::steady_clock::now().time_since_epoch().count()));
 
inline int   randInt (int   lo, int   hi){ return uniform_int_distribution<int>(lo,hi)(rng); }
inline float randF   (float lo, float hi){ return uniform_real_distribution<float>(lo,hi)(rng); }
 
// ═══════════════════════════════════════════════════════════════
//  CONSOLE COLOUR UTILITIES
// ═══════════════════════════════════════════════════════════════
enum class Color : int {
    DEFAULT=7, RED=12, GREEN=10, YELLOW=14,
    BLUE=9, MAGENTA=13, CYAN=11, WHITE=15,
    DARK_RED=4, DARK_GREEN=2, DARK_YELLOW=6, GRAY=8
};
 
void setColor(Color c){
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)c);
#else
    static const unordered_map<int,string> A{
        {7,"\033[0m"},{12,"\033[91m"},{10,"\033[92m"},{14,"\033[93m"},
        {9,"\033[94m"},{13,"\033[95m"},{11,"\033[96m"},{15,"\033[97m"},
        {4,"\033[31m"},{2,"\033[32m"},{6,"\033[33m"},{8,"\033[90m"}
    };
    auto it = A.find((int)c);
    if(it != A.end()) cout << it->second;
#endif
}
void resetColor(){ setColor(Color::DEFAULT); }
 
void print   (const string& s, Color c=Color::WHITE){ setColor(c); cout << s; resetColor(); }
void println (const string& s, Color c=Color::WHITE){ print(s+"\n",c); }
void clearScr(){ system(CLEAR_CMD); }
void pause_ms(int ms=700){ this_thread::sleep_for(chrono::milliseconds(ms)); }
 
void printSep(const string& ch="═", int len=62, Color c=Color::GRAY){
    setColor(c);
    for(int i=0;i<len;i++) cout << ch;
    cout << '\n';
    resetColor();
}
 
// Template: draw a boxed header for any printable type
template<typename T>
void displayBox(const T& title, Color c=Color::CYAN){
    ostringstream oss; oss << title;
    string s = oss.str();
    setColor(c);
    cout << "╔";
    for(size_t i=0;i<s.size()+4;i++) cout << "═";
    cout << "╗\n║  " << s << "  ║\n╚";
    for(size_t i=0;i<s.size()+4;i++) cout << "═";
    cout << "╝\n";
    resetColor();
}
 
// Inline HP / MP / XP progress bar
void drawBar(int cur, int mx, int len, Color full, Color empty){
    int f = mx>0 ? (int)((float)cur/mx*len) : 0;
    f = max(0, min(f, len));
    setColor(full);
    for(int i=0;i<f;i++)     cout << "█";
    setColor(empty);
    for(int i=0;i<len-f;i++) cout << "░";
    resetColor();
}
 
void pressEnter(){
    println("\n  Press ENTER to continue...", Color::GRAY);
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    cin.get();
}
 
void safeFlush(){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
}
 
// ═══════════════════════════════════════════════════════════════
//  CUSTOM EXCEPTION HIERARCHY
// ═══════════════════════════════════════════════════════════════
class GameException       : public runtime_error {
public: explicit GameException(const string& m) : runtime_error(m){} };
class InvalidInput        : public GameException {
public: explicit InvalidInput(const string& m) : GameException("Invalid input: "+m){} };
class InventoryFull       : public GameException {
public: InventoryFull() : GameException("Inventory is full! (max 10 items)"){} };
class SaveException       : public GameException {
public: explicit SaveException(const string& m) : GameException("Save error: "+m){} };
class InsufficientFunds   : public GameException {
public: InsufficientFunds() : GameException("Not enough gold!"){} };
class InsufficientMP      : public GameException {
public: InsufficientMP() : GameException("Not enough MP!"){} };
 
// ═══════════════════════════════════════════════════════════════
//  SCOPED ENUMS
// ═══════════════════════════════════════════════════════════════
enum class ClassType    { WARRIOR, MAGE, ROGUE, PALADIN };
enum class StatusEffect { NONE, BURN, POISON, STUN, FROZEN, BLESSED };
enum class SkillType    { ATTACK, HEAL, BUFF, ULTIMATE };
enum class ItemType     { WEAPON, ARMOR, POTION };
 
string statusName (StatusEffect s){
    switch(s){
        case StatusEffect::BURN   : return "BURN";
        case StatusEffect::POISON : return "POISON";
        case StatusEffect::STUN   : return "STUN";
        case StatusEffect::FROZEN : return "FROZEN";
        case StatusEffect::BLESSED: return "BLESSED";
        default                   : return "NONE";
    }
}
Color statusColor(StatusEffect s){
    switch(s){
        case StatusEffect::BURN   : return Color::RED;
        case StatusEffect::POISON : return Color::DARK_GREEN;
        case StatusEffect::STUN   : return Color::YELLOW;
        case StatusEffect::FROZEN : return Color::CYAN;
        case StatusEffect::BLESSED: return Color::MAGENTA;
        default                   : return Color::WHITE;
    }
}
 
// ═══════════════════════════════════════════════════════════════
//  CLASS SPRITE SYSTEM  — ASCII art action figures per class
// ═══════════════════════════════════════════════════════════════
using SpriteLine = pair<string,Color>;
using Sprite     = vector<SpriteLine>;
 
Sprite getClassSprite(ClassType ct){
    switch(ct){
    case ClassType::WARRIOR: return {
        {"        O        ", Color::WHITE   },
        {"       /|\\       ", Color::WHITE   },
        {"     .-]|[-.     ", Color::RED     },
        {"    [========]   ", Color::RED     },
        {"    [  \\|/  ]   ", Color::RED     },
        {"    [========]   ", Color::RED     },
        {"      /   \\      ", Color::WHITE   },
        {"    _/     \\_    ", Color::RED     },
        {"   [_]     [_]   ", Color::GRAY    },
        {"     WARRIOR      ", Color::RED     },
    };
    case ClassType::MAGE: return {
        {"    *   O   *    ", Color::CYAN    },
        {"   * \\  |  / *  ", Color::BLUE    },
        {"    .~(   )~.    ", Color::CYAN    },
        {"    (  ***  )    ", Color::BLUE    },
        {"    '~(   )~'    ", Color::CYAN    },
        {"        |        ", Color::BLUE    },
        {"       /|\\       ", Color::BLUE    },
        {"      * | *      ", Color::CYAN    },
        {"     *  *  *     ", Color::MAGENTA },
        {"       MAGE       ", Color::BLUE    },
    };
    case ClassType::ROGUE: return {
        {"       /O\\       ", Color::DARK_GREEN},
        {"     _/ | \\_     ", Color::DARK_GREEN},
        {"    /   |   \\    ", Color::GREEN   },
        {"  <-----|----->  ", Color::GREEN    },
        {"    \\   |   /    ", Color::DARK_GREEN},
        {"     |  |  |     ", Color::DARK_GREEN},
        {"    >|  |  |<    ", Color::GREEN    },
        {"    /|  |  |\\    ", Color::DARK_GREEN},
        {"   /_|__|__|_\\   ", Color::DARK_GREEN},
        {"      ROGUE       ", Color::DARK_GREEN},
    };
    case ClassType::PALADIN: return {
        {"    * {O} *      ", Color::YELLOW  },
        {"   *  -+- *      ", Color::YELLOW  },
        {"    .-[+]-.      ", Color::YELLOW  },
        {"   [=======]     ", Color::YELLOW  },
        {"   |  [ + ]|     ", Color::WHITE   },
        {"   |  [   ]|     ", Color::YELLOW  },
        {"   [=======]     ", Color::YELLOW  },
        {"     /   \\       ", Color::WHITE   },
        {"    /  +  \\      ", Color::YELLOW  },
        {"     PALADIN      ", Color::YELLOW  },
    };
    }
    return {};
}
 
void printSprite(ClassType ct, const string& pad="  "){
    for(auto& [line,color] : getClassSprite(ct)){
        print(pad, Color::WHITE);
        println(line, color);
    }
}
 
// Print all 4 sprites side-by-side (each column 20 chars wide)
void printAllSprites(){
    const vector<ClassType> classes={ClassType::WARRIOR, ClassType::MAGE,
                                     ClassType::ROGUE,   ClassType::PALADIN};
    vector<Sprite> sprites;
    for(auto c : classes) sprites.push_back(getClassSprite(c));
    size_t rows = sprites[0].size();
    for(size_t r = 0; r < rows; r++){
        cout << "  ";
        for(int c = 0; c < 4; c++){
            string line = sprites[c][r].first;
            while(line.size() < 20) line += ' ';
            setColor(sprites[c][r].second);
            cout << line;
        }
        resetColor();
        cout << "\n";
    }
}
 
// Pad string to fixed width (for side-by-side layout)
string padRight(const string& s, size_t w){
    if(s.size()>=w) return s.substr(0,w);
    return s + string(w-s.size(),' ');
}
 
// Compact 7-line D&D-style battle sprite shown in the combat HUD
// Each figure has class-defining equipment clearly visible
Sprite getBattleSprite(ClassType ct){
    switch(ct){
    // Warrior: heavy helm, raised sword(S) on left, tower shield(#) on right
    case ClassType::WARRIOR: return {
        {"    (O)     ", Color::WHITE  },
        {" S--[|]--#  ", Color::RED   },
        {"  [=====]   ", Color::RED   },
        {"  |  +  |   ", Color::RED   },
        {"  [=====]   ", Color::RED   },
        {"  /     \\   ", Color::WHITE },
        {" [_]   [_]  ", Color::RED   },
    };
    // Mage: pointy hat, flowing robes, staff(*) with glowing orb(o)
    case ClassType::MAGE: return {
        {"   /\\(O)/\\  ", Color::CYAN   },
        {"  ( \\|/ )   ", Color::BLUE   },
        {"  (|***|)   ", Color::CYAN   },
        {"   \\| |/    ", Color::BLUE   },
        {"    | |     ", Color::BLUE   },
        {"    | *     ", Color::CYAN   },
        {"   _|_o_    ", Color::MAGENTA},
    };
    // Rogue: deep hood, crossed daggers(><), dark cloak
    case ClassType::ROGUE: return {
        {"   .(O).    ", Color::DARK_GREEN},
        {"  {/   \\}  ", Color::GREEN     },
        {" </     \\>  ", Color::GREEN     },
        {"  | >-< |   ", Color::DARK_GREEN},
        {"  |     |   ", Color::DARK_GREEN},
        {"  /     \\   ", Color::DARK_GREEN},
        {" /_\\   /_\\  ", Color::GREEN    },
    };
    // Paladin: halo(*), holy armor, cross(+) emblem, holy sword(|) raised
    case ClassType::PALADIN: return {
        {"  *{O}*     ", Color::YELLOW},
        {"  -|+|-     ", Color::YELLOW},
        {" [=====]    ", Color::YELLOW},
        {" |  +  |    ", Color::WHITE },
        {" [=====]    ", Color::YELLOW},
        {"  /   \\     ", Color::WHITE },
        {" / (+) \\    ", Color::YELLOW},
    };
    }
    return {};
}
 
// ── Enemy sprites — matched by name keyword ──
Sprite getEnemySprite(const string& rawName){
    string n = rawName;
    transform(n.begin(),n.end(),n.begin(),::tolower);
    auto has=[&](const string& k){ return n.find(k)!=string::npos; };
 
    if(has("goblin")||has("grunt")) return {
        {"   .o.    ", Color::GREEN     },
        {"  /|V|\\   ", Color::GREEN    },
        {" (>o_o<)  ", Color::GREEN     },
        {"  \\_W_/   ", Color::DARK_GREEN},
        {"   /|\\    ", Color::DARK_GREEN},
        {"  / | \\   ", Color::DARK_GREEN},
        {" /  |  \\  ", Color::GREEN    },
    };
    if(has("bat")||has("cave")) return {
        {" /\\~O~/\\  ", Color::GRAY     },
        {"/ \\-|-/ \\  ", Color::DARK_GREEN},
        {"|  \\|/  |  ", Color::GRAY     },
        {" \\  V  /   ", Color::DARK_GREEN},
        {"  \\ | /    ", Color::GRAY     },
        {"   \\|/     ", Color::GRAY     },
        {"    V      ", Color::DARK_GREEN},
    };
    if(has("skeleton")||has("archer")||has("lich")||has("undead")) return {
        {"  _/~\\_   ", Color::GRAY  },
        {"  |X X|   ", Color::WHITE },
        {"  \\___/   ", Color::GRAY  },
        {"  /| |\\   ", Color::WHITE },
        {" / | | \\  ", Color::GRAY  },
        {"   /   \\  ", Color::WHITE },
        {"  /     \\ ", Color::GRAY  },
    };
    if(has("slime")) return {
        {" .~~~~~.  ", Color::GREEN     },
        {"/ o   o \\  ", Color::GREEN    },
        {"|  ~~~   | ", Color::DARK_GREEN},
        {"|  ___   | ", Color::GREEN     },
        {" \\_____/  ", Color::GREEN     },
        {"  .   .   ", Color::DARK_GREEN},
        {" / ~~~ \\  ", Color::GREEN    },
    };
    if(has("orc")||has("berserk")||has("wolf")) return {
        {"  .(O).   ", Color::RED  },
        {" /|===|\\  ", Color::RED  },
        {" [=====]  ", Color::RED  },
        {" |  W  |  ", Color::WHITE},
        {" [=====]  ", Color::RED  },
        {"  /   \\   ", Color::WHITE},
        {" /     \\  ", Color::RED  },
    };
    if(has("cultist")||has("witch")) return {
        {"  ~(O)~   ", Color::MAGENTA   },
        {" /|~~~|\\  ", Color::BLUE      },
        {"(#######) ", Color::BLUE      },
        {" \\|~~~|/  ", Color::MAGENTA   },
        {"   |||    ", Color::BLUE      },
        {"   /|\\    ", Color::MAGENTA   },
        {"  / * \\   ", Color::CYAN      },
    };
    if(has("golem")||has("stone")) return {
        {" [.O O.]  ", Color::GRAY },
        {" [=====]  ", Color::GRAY },
        {"[=======] ", Color::GRAY },
        {"| [===] | ", Color::WHITE},
        {"[=======] ", Color::GRAY },
        {" /     \\  ", Color::GRAY },
        {"[_]   [_] ", Color::GRAY },
    };
    if(has("drake")||has("dragon")||has("wyrm")||has("hatchling")) return {
        {" /\\_O_/\\ ", Color::RED   },
        {"(>O   O<) ", Color::RED   },
        {" \\-www-/ ", Color::YELLOW},
        {" /|XXX|\\ ", Color::RED   },
        {"/\\ |X| /\\ ", Color::RED  },
        {" \\/ | \\/ ", Color::YELLOW},
        {"   /   \\  ", Color::RED   },
    };
    if(has("void")||has("assassin")||has("walker")||has("harvest")||has("shadow")) return {
        {"  >~O~<   ", Color::GRAY      },
        {" /|||||\\  ", Color::DARK_GREEN},
        {"(#######) ", Color::DARK_GREEN},
        {" \\|||||/  ", Color::GRAY      },
        {"   |||    ", Color::DARK_GREEN},
        {"   /|\\    ", Color::GRAY      },
        {"  /   \\   ", Color::DARK_GREEN},
    };
    // Named bosses
    if(has("grumtusk")||has("ironclad")||has("overlord")) return {
        {" .(OOO).  ", Color::RED   },
        {"/|=====|\\ ", Color::RED   },
        {"[=======] ", Color::RED   },
        {"| [===] | ", Color::WHITE },
        {"[=======] ", Color::RED   },
        {" /     \\  ", Color::WHITE },
        {"/ [_+_] \\ ", Color::RED   },
    };
    if(has("umbra")||has("abyss")||has("chaos")||has("crimson")||has("elder")) return {
        {"*[.O.]*   ", Color::MAGENTA},
        {" [|||]    ", Color::MAGENTA},
        {"[=====]   ", Color::MAGENTA},
        {"| >+< |   ", Color::WHITE  },
        {"[=====]   ", Color::MAGENTA},
        {" /   \\    ", Color::WHITE  },
        {"/  X  \\   ", Color::MAGENTA},
    };
    // Generic fallback
    return {
        {"   (O)    ", Color::RED  },
        {"   /|\\    ", Color::RED  },
        {"  [===]   ", Color::RED  },
        {"  |   |   ", Color::RED  },
        {"  [===]   ", Color::RED  },
        {"  /   \\   ", Color::WHITE},
        {" /     \\  ", Color::RED  },
    };
}
 
// ═══════════════════════════════════════════════════════════════
//  STATS  (Operator Overloading)
// ═══════════════════════════════════════════════════════════════
struct Stats {
    int hp=0, maxHp=0;
    int mp=0, maxMp=0;
    int str=0, def=0, agi=0, intel=0, luck=0;
    int statPts=0;
 
    Stats operator+(const Stats& o) const {
        return {hp+o.hp, maxHp+o.maxHp, mp+o.mp, maxMp+o.maxMp,
                str+o.str, def+o.def, agi+o.agi, intel+o.intel,
                luck+o.luck, statPts+o.statPts};
    }
    Stats& operator+=(const Stats& o){ *this = *this + o; return *this; }
    bool   operator==(const Stats& o) const {
        return hp==o.hp && maxHp==o.maxHp && str==o.str;
    }
    void clamp(){
        hp  = max(0, min(hp,  maxHp));
        mp  = max(0, min(mp,  maxMp));
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  ITEM HIERARCHY  (Abstract → Weapon, Armor, Potion)
// ═══════════════════════════════════════════════════════════════
class Item {
protected:
    string  name, desc;
    int     value;
    ItemType type;
public:
    Item(string n, string d, int v, ItemType t)
        : name(move(n)), desc(move(d)), value(v), type(t){}
    virtual ~Item() = default;
 
    const string& getName()  const { return name;  }
    const string& getDesc()  const { return desc;  }
    int           getValue() const { return value; }
    ItemType      getType()  const { return type;  }
 
    virtual Stats getStatBonus()      const { return {}; }
    virtual int   getHealAmt()        const { return 0;  }
    virtual int   getMpRestoreAmt()   const { return 0;  }
    virtual void  display()           const = 0;
    virtual string serialize()        const = 0;
};
 
class Weapon : public Item {
    int atkBonus;
public:
    Weapon(string n, string d, int v, int atk)
        : Item(move(n),move(d),v,ItemType::WEAPON), atkBonus(atk){}
 
    int   getAtkBonus()           const { return atkBonus; }
    Stats getStatBonus()          const override { Stats s{}; s.str=atkBonus; return s; }
 
    void display() const override {
        print("  [⚔  WEAPON] ", Color::RED);
        print(name, Color::WHITE);
        print("  ATK +"+to_string(atkBonus), Color::YELLOW);
        println("  │  "+desc+" │ Worth: "+to_string(value)+"g", Color::GRAY);
    }
    string serialize() const override {
        return "WEAPON|"+name+"|"+desc+"|"+to_string(value)+"|"+to_string(atkBonus);
    }
};
 
class Armor : public Item {
    int defBonus;
public:
    Armor(string n, string d, int v, int df)
        : Item(move(n),move(d),v,ItemType::ARMOR), defBonus(df){}
 
    int   getDefBonus()           const { return defBonus; }
    Stats getStatBonus()          const override { Stats s{}; s.def=defBonus; return s; }
 
    void display() const override {
        print("  [🛡  ARMOR]  ", Color::BLUE);
        print(name, Color::WHITE);
        print("  DEF +"+to_string(defBonus), Color::YELLOW);
        println("  │  "+desc+" │ Worth: "+to_string(value)+"g", Color::GRAY);
    }
    string serialize() const override {
        return "ARMOR|"+name+"|"+desc+"|"+to_string(value)+"|"+to_string(defBonus);
    }
};
 
class Potion : public Item {
    int healAmt, mpAmt;
public:
    Potion(string n, string d, int v, int heal, int mp=0)
        : Item(move(n),move(d),v,ItemType::POTION), healAmt(heal), mpAmt(mp){}
 
    int  getHealAmt()      const override { return healAmt; }
    int  getMpRestoreAmt() const override { return mpAmt;   }
 
    void display() const override {
        print("  [🧪 POTION]  ", Color::GREEN);
        print(name, Color::WHITE);
        if(healAmt>0) print("  HP +"+to_string(healAmt), Color::GREEN);
        if(mpAmt  >0) print("  MP +"+to_string(mpAmt),   Color::BLUE);
        println("  │  "+desc+" │ Worth: "+to_string(value)+"g", Color::GRAY);
    }
    string serialize() const override {
        return "POTION|"+name+"|"+desc+"|"+to_string(value)+
               "|"+to_string(healAmt)+"|"+to_string(mpAmt);
    }
};
 
// ───── Item Factory  (Factory Pattern) ─────
class ItemFactory {
public:
    static shared_ptr<Item> createWeapon(const string& tier){
        using T = tuple<string,string,int,int>;
        static const vector<T> low={
            {"Rusty Sword","A tarnished blade",50,5},
            {"Iron Dagger","Fast and light",80,8}
        };
        static const vector<T> mid={
            {"Steel Longsword","Reliable and sharp",250,18},
            {"Enchanted Rapier","Crackles with energy",400,26},
            {"Shadow Bow","Carved from dark wood",350,22}
        };
        static const vector<T> high={
            {"Dragon Fang","A dragon's shed tooth",950,42},
            {"Void Reaper","Cuts through reality",1800,62},
            {"Celestial Edge","Forged in starlight",2200,78}
        };
        const auto& pool = (tier=="high") ? high : (tier=="mid") ? mid : low;
        const auto& [n,d,v,a] = pool[randInt(0,(int)pool.size()-1)];
        return make_shared<Weapon>(n,d,v,a);
    }
    static shared_ptr<Item> createArmor(const string& tier){
        using T = tuple<string,string,int,int>;
        static const vector<T> low={
            {"Cloth Rags","Barely protective",30,3},
            {"Leather Vest","Simple but sturdy",90,7}
        };
        static const vector<T> mid={
            {"Chain Mail","Interlocked steel rings",220,16},
            {"Knight's Breastplate","Heavy duty",480,25},
            {"Shadow Cloak","Light and deflecting",300,20}
        };
        static const vector<T> high={
            {"Dragon Scale Mail","Impervious to fire",900,40},
            {"Void Aegis","Absorbs dark energy",1700,58},
            {"Celestial Plate","Blessed by the gods",2100,72}
        };
        const auto& pool = (tier=="high") ? high : (tier=="mid") ? mid : low;
        const auto& [n,d,v,a] = pool[randInt(0,(int)pool.size()-1)];
        return make_shared<Armor>(n,d,v,a);
    }
    static shared_ptr<Item> createPotion(bool large=false){
        if(large) return make_shared<Potion>("Mega Elixir","Restores HP & MP",200,150,80);
        return make_shared<Potion>("Health Potion","Restores HP",60,50,0);
    }
    static shared_ptr<Item> createMpPotion(){
        return make_shared<Potion>("Mana Flask","Restores MP",60,0,60);
    }
    static shared_ptr<Item> deserialize(const string& line){
        if(line.empty()||line=="NONE") return nullptr;
        istringstream ss(line);
        string type,name,desc,valS;
        getline(ss,type,'|'); getline(ss,name,'|');
        getline(ss,desc,'|'); getline(ss,valS,'|');
        int val = stoi(valS);
        if(type=="WEAPON"){
            string a; getline(ss,a,'|');
            return make_shared<Weapon>(name,desc,val,stoi(a));
        }
        if(type=="ARMOR"){
            string d; getline(ss,d,'|');
            return make_shared<Armor>(name,desc,val,stoi(d));
        }
        if(type=="POTION"){
            string h,m; getline(ss,h,'|'); getline(ss,m,'|');
            return make_shared<Potion>(name,desc,val,stoi(h),stoi(m));
        }
        throw GameException("Unknown item type: "+type);
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  SKILL HIERARCHY  (Abstract → Attack, Heal, Buff, Ultimate)
// ═══════════════════════════════════════════════════════════════
struct SkillEffect {
    int   damage  = 0;
    int   heal    = 0;
    int   mpCost  = 0;
    StatusEffect status  = StatusEffect::NONE;
    int   statusChance   = 0;   // 0-100 %
};
 
class Skill {
protected:
    string    name, desc;
    int       mpCost;
    SkillType type;
    int       lvlReq;
public:
    Skill(string n, string d, int mp, SkillType t, int lr=1)
        : name(move(n)), desc(move(d)), mpCost(mp), type(t), lvlReq(lr){}
    virtual ~Skill() = default;
 
    const string& getName()  const { return name;   }
    const string& getDesc()  const { return desc;   }
    int           getMpCost()const { return mpCost; }
    SkillType     getType()  const { return type;   }
    int           getLvlReq()const { return lvlReq; }
 
    virtual SkillEffect calc(const Stats& s) const = 0;
 
    void display() const {
        Color c = (type==SkillType::ULTIMATE) ? Color::MAGENTA :
                  (type==SkillType::ATTACK)   ? Color::RED :
                  (type==SkillType::HEAL)      ? Color::GREEN : Color::YELLOW;
        print("  ["+name+"]", c);
        print("  MP:"+to_string(mpCost), Color::BLUE);
        print("  Lv:"+to_string(lvlReq), Color::GRAY);
        println("  "+desc, Color::GRAY);
    }
};
 
class AttackSkill : public Skill {
    float mult;
    StatusEffect se;
    int   seChance;
public:
    AttackSkill(string n, string d, int mp, float m,
                StatusEffect s=StatusEffect::NONE, int sc=0, int lr=1)
        : Skill(move(n),move(d),mp,SkillType::ATTACK,lr),
          mult(m), se(s), seChance(sc){}
    SkillEffect calc(const Stats& s) const override {
        return {(int)(s.str*mult), 0, mpCost, se, seChance};
    }
};
 
class HealSkill : public Skill {
    float mult;
public:
    HealSkill(string n, string d, int mp, float m, int lr=1)
        : Skill(move(n),move(d),mp,SkillType::HEAL,lr), mult(m){}
    SkillEffect calc(const Stats& s) const override {
        int h = (int)(s.intel*mult + s.maxHp*0.12f);
        return {0, h, mpCost, StatusEffect::BLESSED, 100};
    }
};
 
class UltimateSkill : public Skill {
    float mult;
    StatusEffect se;
public:
    UltimateSkill(string n, string d, int mp, float m,
                  StatusEffect s=StatusEffect::STUN, int lr=1)
        : Skill(move(n),move(d),mp,SkillType::ULTIMATE,lr), mult(m), se(s){}
    SkillEffect calc(const Stats& s) const override {
        int d = (int)(s.str*mult + s.intel*(mult*0.4f));
        return {d, 0, mpCost, se, 70};
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  CHARACTER (Abstract Base)  →  Warrior, Mage, Rogue, Paladin
// ═══════════════════════════════════════════════════════════════
class Character {
protected:
    string name;
    ClassType  ct;
    Stats      stats;
    int  level=1, xp=0, xpNext=100, gold=100, floor=1;
    StatusEffect status=StatusEffect::NONE;
    int          statusTurns=0;
 
    vector<shared_ptr<Skill>> skills;
    vector<shared_ptr<Item>>  inventory;
    shared_ptr<Weapon> eqWeapon;
    shared_ptr<Armor>  eqArmor;
 
    // Observer pattern – event callbacks
    vector<function<void(const string&)>> listeners;
 
public:
    Character(string n, ClassType c, Stats s)
        : name(move(n)), ct(c), stats(s){}
    virtual ~Character() = default;
 
    // Pure virtual interface — each class defines its identity
    virtual void        initSkills()          = 0;
    virtual void        onLevelUp()           = 0;   // class-specific growth
    virtual string      getClassName()  const = 0;
    virtual Color       getClassColor() const = 0;
 
    // ── Accessors ──
    ClassType     getClassType()   const { return ct;    }
    const string& getName()  const { return name;  }
    void          setName(const string& n){ name=n; }
    int           getLevel() const { return level; }
    int           getXp()    const { return xp;    }
    int           getXpNext()const { return xpNext;}
    int           getGold()  const { return gold;  }
    int           getFloor() const { return floor; }
    void          setFloor(int f)  { floor=f;      }
    Stats&        getStats()       { return stats;  }
    const Stats&  getStats() const { return stats;  }
    vector<shared_ptr<Skill>>&       getSkills()   { return skills;    }
    vector<shared_ptr<Item>>&        getInv()      { return inventory; }
    StatusEffect  getStatus()       const { return status;      }
    int           getStatusTurns()  const { return statusTurns; }
    bool          isAlive()         const { return stats.hp>0;  }
    bool          isStunned()       const { return status==StatusEffect::STUN && statusTurns>0; }
 
    void addGold  (int g){ gold+=g; }
    void spendGold(int g){
        if(g>gold) throw InsufficientFunds();
        gold-=g;
    }
 
    // Observer helpers
    void addListener(function<void(const string&)> cb){ listeners.push_back(move(cb)); }
    void notify     (const string& ev){ for(auto& f:listeners) f(ev); }
 
    // Effective stats = base + equipment bonuses
    virtual Stats effectiveStats() const {
        Stats eff = stats;
        if(eqWeapon) eff += eqWeapon->getStatBonus();
        if(eqArmor ) eff += eqArmor ->getStatBonus();
        return eff;
    }
 
    // ── Status Effects ──
    void applyStatus(StatusEffect se, int turns){
        status=se; statusTurns=turns;
    }
    void clearStatus(){ status=StatusEffect::NONE; statusTurns=0; }
 
    // Returns damage dealt by status; decrements turns
    int tickStatus(){
        if(status==StatusEffect::NONE || statusTurns<=0){ clearStatus(); return 0; }
        StatusEffect prev = status;
        int dmg=0;
        if(prev==StatusEffect::BURN  ) dmg=max(1,(int)(stats.maxHp*0.08f));
        if(prev==StatusEffect::POISON) dmg=max(1,(int)(stats.maxHp*0.05f));
        stats.hp -= dmg;
        statusTurns--;
        if(statusTurns<=0) clearStatus();
        stats.clamp();
        return dmg;
    }
 
    // ── XP & Levelling ──
    bool gainXp(int amt){
        xp += amt;
        bool lvdUp = false;
        while(xp >= xpNext){
            xp    -= xpNext;
            level ++;
            xpNext = (int)(100 * pow(level, 1.55));
            stats.statPts += 5;
            stats.maxHp   += 12;
            stats.maxMp   += 7;
            stats.hp = stats.maxHp;
            stats.mp = stats.maxMp;
            onLevelUp();      // class-specific bonus
            notify("LEVEL_UP");
            lvdUp = true;
        }
        return lvdUp;
    }
 
    // ── Stat Distribution ──
    void distributeStat(const string& stat, int pts){
        if(pts<=0)           throw InvalidInput("Points must be positive");
        if(pts>stats.statPts)throw InvalidInput("Not enough stat points");
        string s = stat;
        transform(s.begin(),s.end(),s.begin(),::toupper);
        if     (s=="STR"){ stats.str   +=pts; }
        else if(s=="DEF"){ stats.def   +=pts; }
        else if(s=="AGI"){ stats.agi   +=pts; }
        else if(s=="INT"){ stats.intel +=pts; }
        else if(s=="LCK"){ stats.luck  +=pts; }
        else if(s=="HP" ){ stats.maxHp +=pts*10; stats.hp+=pts*10; }
        else if(s=="MP" ){ stats.maxMp +=pts*5;  stats.mp+=pts*5;  }
        else throw InvalidInput("Unknown stat '"+stat+"'. Use STR/DEF/AGI/INT/LCK/HP/MP");
        stats.statPts -= pts;
    }
 
    // ── Equipment ──
    void equip(shared_ptr<Weapon> w){ eqWeapon=w; }
    void equip(shared_ptr<Armor>  a){ eqArmor =a; }
    shared_ptr<Weapon> getWeapon() const { return eqWeapon; }
    shared_ptr<Armor>  getArmor()  const { return eqArmor;  }
 
    // ── Inventory ──
    void addItem(shared_ptr<Item> item){
        if((int)inventory.size()>=10) throw InventoryFull();
        inventory.push_back(item);
    }
 
    // Use item by inventory index; returns result string
    string useItem(int idx){
        if(idx<0||idx>=(int)inventory.size())
            throw InvalidInput("No item at that slot");
        auto& item = inventory[idx];
        string res;
        if(item->getType()==ItemType::POTION){
            int h=item->getHealAmt(), m=item->getMpRestoreAmt();
            stats.hp = min(stats.maxHp, stats.hp+h);
            stats.mp = min(stats.maxMp, stats.mp+m);
            res = "Used "+item->getName();
            if(h) res+="  (HP +"+to_string(h)+")";
            if(m) res+="  (MP +"+to_string(m)+")";
            inventory.erase(inventory.begin()+idx);
        } else if(item->getType()==ItemType::WEAPON){
            auto w = dynamic_pointer_cast<Weapon>(item);
            if(w){ equip(w); res="Equipped "+item->getName(); }
            inventory.erase(inventory.begin()+idx);
        } else if(item->getType()==ItemType::ARMOR){
            auto a = dynamic_pointer_cast<Armor>(item);
            if(a){ equip(a); res="Equipped "+item->getName(); }
            inventory.erase(inventory.begin()+idx);
        } else {
            res = "Cannot use "+item->getName()+" here.";
        }
        return res;
    }
 
    // ── Display ──
    void displayStats() const {
        Stats eff = effectiveStats();
        println("", Color::WHITE);
 
        // Side-by-side: sprite on left, stats on right
        auto sprite = getClassSprite(ct);
        vector<string> statLines;
        statLines.push_back("");
        statLines.push_back("  " + name + "  [" + getClassName() + "]");
        statLines.push_back("  Level " + to_string(level));
        statLines.push_back("  ─────────────────────");
        {
            int bLen=16;
            ostringstream hp; hp<<"  HP [";
            for(int i=0;i<bLen;i++) hp<<(i < (stats.maxHp>0?(int)((float)stats.hp/stats.maxHp*bLen):0) ? '#':'-');
            hp<<"] "+to_string(stats.hp)+"/"+to_string(stats.maxHp);
            statLines.push_back(hp.str());
        }
        {
            int bLen=16;
            ostringstream mp; mp<<"  MP [";
            for(int i=0;i<bLen;i++) mp<<(i < (stats.maxMp>0?(int)((float)stats.mp/stats.maxMp*bLen):0) ? '=':'-');
            mp<<"] "+to_string(stats.mp)+"/"+to_string(stats.maxMp);
            statLines.push_back(mp.str());
        }
        {
            int bLen=16;
            ostringstream xpBar; xpBar<<"  XP [";
            for(int i=0;i<bLen;i++) xpBar<<(i < (xpNext>0?(int)((float)xp/xpNext*bLen):0) ? '*':'-');
            xpBar<<"] "+to_string(xp)+"/"+to_string(xpNext);
            statLines.push_back(xpBar.str());
        }
        statLines.push_back("  ─────────────────────");
        statLines.push_back("  STR:" +to_string(eff.str)+"  DEF:"+to_string(eff.def)+"  AGI:"+to_string(eff.agi));
        statLines.push_back("  INT:"+to_string(eff.intel)+"  LCK:"+to_string(eff.luck)+"  Gold:"+to_string(gold)+"g");
 
        size_t rows = max(sprite.size(), statLines.size());
        for(size_t i=0;i<rows;i++){
            // Sprite column (22 chars wide)
            if(i < sprite.size()){
                string sp = sprite[i].first;
                while(sp.size()<22) sp+=' ';
                setColor(sprite[i].second);
                cout << "  " << sp;
                resetColor();
            } else {
                cout << string(24,' ');
            }
            // Stats column
            if(i < statLines.size()){
                Color sc = (i==1) ? getClassColor() :
                           (i==4) ? Color::GREEN :
                           (i==5) ? Color::BLUE  :
                           (i==6) ? Color::YELLOW :
                           (i==8 || i==9) ? Color::CYAN : Color::WHITE;
                println(statLines[i], sc);
            } else {
                cout << "\n";
            }
        }
 
        printSep("-",54,Color::GRAY);
        print  ("  Floor: "+to_string(floor), Color::WHITE);
        print  ("   |  Stat Points free: ", Color::WHITE);
        println(to_string(stats.statPts), Color::MAGENTA);
        if(status!=StatusEffect::NONE)
            print("  STATUS: ["+statusName(status)+" "+to_string(statusTurns)+" turns]\n",
                  statusColor(status));
        if(eqWeapon)
            println("  Weapon: "+eqWeapon->getName()+"  (ATK +"+to_string(eqWeapon->getAtkBonus())+")",
                    Color::RED);
        if(eqArmor)
            println("  Armor:  "+eqArmor->getName()+"  (DEF +"+to_string(eqArmor->getDefBonus())+")",
                    Color::BLUE);
    }
 
    void displayInventory() const {
        if(inventory.empty()){ println("  (inventory empty)", Color::GRAY); return; }
        println("\n  ── INVENTORY ("+to_string(inventory.size())+"/10) ──", Color::YELLOW);
        for(int i=0;i<(int)inventory.size();i++){
            print("  "+to_string(i+1)+". ", Color::GRAY);
            inventory[i]->display();
        }
    }
 
    // ── Serialisation ──
    virtual string serialize() const {
        ostringstream o;
        o << name << "\n" << (int)ct << "\n"
          << level<<" "<<xp<<" "<<xpNext<<" "<<gold<<" "<<floor<<"\n"
          << stats.hp<<" "<<stats.maxHp<<" "<<stats.mp<<" "<<stats.maxMp<<" "
          << stats.str<<" "<<stats.def<<" "<<stats.agi<<" "<<stats.intel<<" "
          << stats.luck<<" "<<stats.statPts<<"\n"
          << inventory.size() << "\n";
        for(auto& it: inventory) o << it->serialize() << "\n";
        o << (eqWeapon ? eqWeapon->serialize() : "NONE") << "\n";
        o << (eqArmor  ? eqArmor ->serialize() : "NONE") << "\n";
        return o.str();
    }
 
    void deserializeBody(istream& in){
        in >> level >> xp >> xpNext >> gold >> floor;
        in >> stats.hp >> stats.maxHp >> stats.mp >> stats.maxMp
           >> stats.str >> stats.def >> stats.agi >> stats.intel
           >> stats.luck >> stats.statPts;
        int n; in >> n; in.ignore();
        for(int i=0;i<n;i++){
            string ln; getline(in,ln);
            try{ auto it=ItemFactory::deserialize(ln); if(it) inventory.push_back(it); }
            catch(...){}
        }
        string wL,aL;
        getline(in,wL); getline(in,aL);
        if(wL!="NONE"&&!wL.empty()){
            auto w=dynamic_pointer_cast<Weapon>(ItemFactory::deserialize(wL));
            if(w) eqWeapon=w;
        }
        if(aL!="NONE"&&!aL.empty()){
            auto a=dynamic_pointer_cast<Armor>(ItemFactory::deserialize(aL));
            if(a) eqArmor=a;
        }
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  WARRIOR
// ═══════════════════════════════════════════════════════════════
class Warrior : public Character {
public:
    Warrior(const string& n)
        : Character(n, ClassType::WARRIOR,
          {140,140,25,25,20,14,8,4,6,0}){ initSkills(); }
 
    string getClassName() const override { return "WARRIOR"; }
    Color  getClassColor()const override { return Color::RED; }
 
    void initSkills() override {
        skills.clear();
        skills.push_back(make_shared<AttackSkill>(
            "Power Strike","Heavy blow deals 1.8× STR",8,1.8f));
        skills.push_back(make_shared<AttackSkill>(
            "Shield Bash","Bash + STUN chance",12,1.5f,StatusEffect::STUN,55,1));
        skills.push_back(make_shared<AttackSkill>(
            "Whirlwind","Spinning blade (2.4× STR)",20,2.4f,
            StatusEffect::NONE,0,5));
        skills.push_back(make_shared<UltimateSkill>(
            "Berserker Rage","Unleash raw fury (3.6× STR)",32,3.6f,
            StatusEffect::STUN,8));
        skills.push_back(make_shared<UltimateSkill>(
            "Titan's Wrath","Earth shattering blow (4.8× STR)",52,4.8f,
            StatusEffect::FROZEN,12));
    }
    void onLevelUp() override {
        stats.str   += 2;
        stats.maxHp += 8;
        stats.hp = stats.maxHp;
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  MAGE
// ═══════════════════════════════════════════════════════════════
class Mage : public Character {
public:
    Mage(const string& n)
        : Character(n, ClassType::MAGE,
          {70,70,90,90,4,4,10,24,8,0}){ initSkills(); }
 
    string getClassName() const override { return "MAGE";  }
    Color  getClassColor()const override { return Color::BLUE; }
 
    // Mages deal INT-scaled damage even on normal attacks
    Stats effectiveStats() const override {
        Stats eff = Character::effectiveStats();
        eff.str += eff.intel / 2;
        return eff;
    }
 
    void initSkills() override {
        skills.clear();
        skills.push_back(make_shared<AttackSkill>(
            "Fireball","Scorching blast + BURN (2.3× INT)",10,2.3f,
            StatusEffect::BURN,65));
        skills.push_back(make_shared<AttackSkill>(
            "Ice Lance","Frozen spear + FROZEN (2.0× INT)",12,2.0f,
            StatusEffect::FROZEN,60));
        skills.push_back(make_shared<HealSkill>(
            "Arcane Mend","Restore HP with arcane energy (1.6× INT)",14,1.6f));
        skills.push_back(make_shared<AttackSkill>(
            "Void Bolt","Dark energy + POISON (2.8× INT)",22,2.8f,
            StatusEffect::POISON,60,6));
        skills.push_back(make_shared<UltimateSkill>(
            "Meteor Storm","Rain meteors upon foes (4.2× INT)",46,4.2f,
            StatusEffect::STUN,10));
        skills.push_back(make_shared<UltimateSkill>(
            "Singularity","Tear spacetime (5.5× INT)",65,5.5f,
            StatusEffect::STUN,12));
    }
    void onLevelUp() override {
        stats.intel += 2;
        stats.maxMp += 12;
        stats.mp = stats.maxMp;
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  ROGUE
// ═══════════════════════════════════════════════════════════════
class Rogue : public Character {
public:
    Rogue(const string& n)
        : Character(n, ClassType::ROGUE,
          {90,90,55,55,16,7,22,7,18,0}){ initSkills(); }
 
    string getClassName() const override { return "ROGUE";       }
    Color  getClassColor()const override { return Color::DARK_GREEN; }
 
    void initSkills() override {
        skills.clear();
        skills.push_back(make_shared<AttackSkill>(
            "Backstab","Strike from shadow (2.6× STR)",10,2.6f));
        skills.push_back(make_shared<AttackSkill>(
            "Poison Blade","Coat blade in venom + POISON",12,1.8f,
            StatusEffect::POISON,75));
        skills.push_back(make_shared<AttackSkill>(
            "Shadow Step","Teleport strike + STUN (2.3× STR)",18,2.3f,
            StatusEffect::STUN,55,4));
        skills.push_back(make_shared<UltimateSkill>(
            "Death Mark","Execution mark (3.9× STR)",36,3.9f,
            StatusEffect::POISON,9));
        skills.push_back(make_shared<UltimateSkill>(
            "Assassination","One-shot attempt (5.6× STR)",58,5.6f,
            StatusEffect::STUN,7));
    }
    void onLevelUp() override {
        stats.agi  += 2;
        stats.luck += 1;
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  PALADIN
// ═══════════════════════════════════════════════════════════════
class Paladin : public Character {
public:
    Paladin(const string& n)
        : Character(n, ClassType::PALADIN,
          {110,110,65,65,13,17,8,14,10,0}){ initSkills(); }
 
    string getClassName() const override { return "PALADIN";    }
    Color  getClassColor()const override { return Color::YELLOW; }
 
    void initSkills() override {
        skills.clear();
        skills.push_back(make_shared<AttackSkill>(
            "Holy Strike","Blessed weapon blow (1.7× STR)",8,1.7f));
        skills.push_back(make_shared<HealSkill>(
            "Divine Light","Heal with holy power (2.1× INT)",14,2.1f));
        skills.push_back(make_shared<AttackSkill>(
            "Smite","Divine judgment + BURN (2.1× STR)",18,2.1f,
            StatusEffect::BURN,65));
        skills.push_back(make_shared<HealSkill>(
            "Resurrection Aura","Massive heal (3.0× INT)",28,3.0f,8));
        skills.push_back(make_shared<UltimateSkill>(
            "Armageddon","Heaven's wrath descends (3.8× STR+INT)",44,3.8f,
            StatusEffect::STUN,10));
    }
    void onLevelUp() override {
        stats.def   += 1;
        stats.intel += 1;
        stats.maxHp += 6;
        stats.maxMp += 5;
        stats.hp = stats.maxHp;
        stats.mp = stats.maxMp;
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  ENEMY HIERARCHY  (Enemy → BossEnemy)
// ═══════════════════════════════════════════════════════════════
class Enemy {
protected:
    string name;
    Stats  stats;
    int    level, xpReward, goldReward;
    StatusEffect status=StatusEffect::NONE;
    int          statusTurns=0;
public:
    Enemy(string n, Stats s, int lv, int xp, int gld)
        : name(move(n)), stats(s), level(lv), xpReward(xp), goldReward(gld){}
    virtual ~Enemy() = default;
 
    const string& getName()     const { return name;       }
    Stats&        getStats()          { return stats;       }
    int           getLevel()    const { return level;       }
    int           getXpReward() const { return xpReward;   }
    int           getGldReward()const { return goldReward; }
    bool          isAlive()     const { return stats.hp>0; }
    StatusEffect  getStatus()   const { return status;      }
    int           getStatusTurns() const { return statusTurns; }
 
    bool isStunned() const { return status==StatusEffect::STUN && statusTurns>0; }
 
    void applyStatus(StatusEffect se, int t){ status=se; statusTurns=t; }
 
    int tickStatus(){
        if(status==StatusEffect::NONE||statusTurns<=0){
            status=StatusEffect::NONE; statusTurns=0; return 0;
        }
        int dmg=0;
        if(status==StatusEffect::BURN  ) dmg=max(1,(int)(stats.maxHp*0.08f));
        if(status==StatusEffect::POISON) dmg=max(1,(int)(stats.maxHp*0.05f));
        stats.hp -= dmg;
        statusTurns--;
        if(statusTurns<=0){ status=StatusEffect::NONE; statusTurns=0; }
        stats.clamp();
        return dmg;
    }
 
    virtual int attack(Stats& target){
        int dmg=max(1, stats.str - target.def/2 + randInt(-3,4));
        target.hp -= dmg;
        target.clamp();
        return dmg;
    }
    virtual bool  isBoss()    const { return false;       }
    virtual Color getColor()  const { return Color::RED;  }
 
    void displayStatus() const {
        print("  ["+name, getColor());
        print("  HP:"+to_string(stats.hp)+"/"+to_string(stats.maxHp), Color::WHITE);
        if(status!=StatusEffect::NONE)
            print("  ["+statusName(status)+":"+to_string(statusTurns)+"t]",
                  statusColor(status));
        println("]", Color::WHITE);
    }
};
 
class BossEnemy : public Enemy {
    string title;
    bool   enraged=false;
public:
    BossEnemy(string n, string t, Stats s, int lv, int xp, int gld)
        : Enemy(move(n),s,lv,xp,gld), title(move(t)){}
 
    bool  isBoss()   const override { return true;           }
    Color getColor() const override { return Color::MAGENTA; }
    const string& getTitle() const  { return title;          }
 
    int attack(Stats& target) override {
        // Phase transition at ≤30% HP
        if(!enraged && stats.hp<=(int)(stats.maxHp*0.3f)){
            enraged    = true;
            stats.str  = (int)(stats.str*1.45f);
            println("\n  💀 "+name+" becomes ENRAGED! Attack power surges!",Color::MAGENTA);
        }
        int dmg=max(1, stats.str - target.def/2 + randInt(-5,8));
        // 30% double-hit chance
        if(randInt(1,100)<=30){
            dmg += max(1, stats.str - target.def/2 + randInt(-4,6));
            println("  ⚡ "+name+" strikes TWICE!", Color::MAGENTA);
        }
        target.hp -= dmg; target.clamp();
        return dmg;
    }
 
    void displayStatusBoss() const {
        println("", Color::WHITE);
        print("  💀 [BOSS]  ", Color::MAGENTA);
        println(title+" ─ "+name, Color::WHITE);
        int bLen=30;
        print("  HP [", Color::MAGENTA);
        Color hc = stats.hp>stats.maxHp*0.5f ? Color::GREEN :
                   stats.hp>stats.maxHp*0.25f? Color::YELLOW : Color::RED;
        drawBar(stats.hp,stats.maxHp,bLen,hc,Color::GRAY);
        println("] "+to_string(stats.hp)+"/"+to_string(stats.maxHp), Color::WHITE);
        if(enraged) println("  ⚡ ENRAGED — doubled aggression!", Color::RED);
        if(status!=StatusEffect::NONE)
            println("  ["+statusName(status)+":"+to_string(statusTurns)+"t]",
                    statusColor(status));
    }
};
 
// ───── Enemy Factory ─────
class EnemyFactory {
public:
    // STL algorithm: pool built with lambdas for scaling
    static unique_ptr<Enemy> createRandom(int floor){
        float scale = 1.0f + floor*0.14f;
        auto sc=[&](int b){ return (int)(b*scale); };
 
        using T = tuple<string,Stats,int,int,int>;
        vector<T> pool;
 
        if(floor<=5){
            pool={
                {"Goblin Grunt",   {sc(28),sc(28),0,0,sc(8),sc(3),sc(9),0,sc(5),0}, 1,sc(15),sc(10)},
                {"Cave Bat",       {sc(22),sc(22),0,0,sc(7),sc(2),sc(13),0,sc(4),0},1,sc(12),sc(8)},
                {"Skeleton Archer",{sc(32),sc(32),0,0,sc(9),sc(4),sc(8),0,sc(4),0}, 2,sc(18),sc(14)},
                {"Slime King",     {sc(44),sc(44),0,0,sc(6),sc(9),sc(3),0,sc(2),0}, 2,sc(20),sc(15)},
            };
        } else if(floor<=10){
            pool={
                {"Orc Berserker",  {sc(58),sc(58),0,0,sc(15),sc(6),sc(9),0,sc(5),0},  3,sc(38),sc(28)},
                {"Dark Cultist",   {sc(42),sc(42),0,0,sc(12),sc(4),sc(11),sc(16),sc(6),0},3,sc(42),sc(32)},
                {"Stone Golem",    {sc(88),sc(88),0,0,sc(13),sc(15),sc(4),0,sc(3),0},  4,sc(50),sc(38)},
                {"Cursed Werewolf",{sc(64),sc(64),0,0,sc(18),sc(8),sc(15),0,sc(7),0},  4,sc(55),sc(42)},
            };
        } else if(floor<=15){
            pool={
                {"Shadow Assassin",{sc(74),sc(74),0,0,sc(22),sc(8),sc(22),0,sc(13),0},6,sc(70),sc(55)},
                {"Inferno Drake",  {sc(86),sc(86),0,0,sc(24),sc(6),sc(13),sc(18),sc(8),0},6,sc(75),sc(60)},
                {"Undead Lich",    {sc(68),sc(68),0,0,sc(20),sc(11),sc(8),sc(28),sc(10),0},7,sc(85),sc(70)},
                {"Dragon Hatchling",{sc(105),sc(105),0,0,sc(27),sc(16),sc(10),sc(5),sc(6),0},7,sc(95),sc(75)},
            };
        } else {
            pool={
                {"Void Walker",    {sc(95),sc(95),0,0,sc(30),sc(13),sc(24),sc(22),sc(16),0},10,sc(108),sc(88)},
                {"Chaos Paladin",  {sc(125),sc(125),0,0,sc(32),sc(20),sc(16),sc(10),sc(11),0},10,sc(118),sc(95)},
                {"Soul Harvester", {sc(105),sc(105),0,0,sc(27),sc(16),sc(20),sc(30),sc(13),0},11,sc(128),sc(108)},
                {"Elder Wyrm",     {sc(155),sc(155),0,0,sc(34),sc(22),sc(13),sc(8),sc(8),0}, 12,sc(138),sc(115)},
            };
        }
 
        int idx = randInt(0,(int)pool.size()-1);
        auto& [n,s,lv,xp,gld] = pool[idx];
        return make_unique<Enemy>(n,s,lv,xp,gld);
    }
 
    static unique_ptr<BossEnemy> createBoss(int floor){
        float scale = 1.0f + floor*0.22f;
        auto sc=[&](int b){ return (int)(b*scale); };
 
        using B = tuple<string,string,Stats,int,int>;
        vector<B> bosses={
            {"Grumtusk the Mighty","Warchief of the Depths",
             {sc(210),sc(210),0,0,sc(22),sc(13),sc(10),0,sc(8),0},sc(160),sc(120)},
            {"The Crimson Witch","Sorceress of Eternal Flame",
             {sc(180),sc(180),0,0,sc(16),sc(8),sc(13),sc(27),sc(11),0},sc(215),sc(165)},
            {"Ironclad Overlord","Juggernaut of the Iron Legion",
             {sc(295),sc(295),0,0,sc(27),sc(24),sc(8),sc(5),sc(6),0},sc(295),sc(220)},
            {"Umbra, the Shadow God","Deity of Darkness",
             {sc(360),sc(360),0,0,sc(33),sc(22),sc(22),sc(22),sc(16),0},sc(420),sc(320)},
            {"The Abyss Incarnate","Ruler of the Eternal Depth",
             {sc(520),sc(520),0,0,sc(40),sc(28),sc(28),sc(32),sc(22),0},sc(650),sc(530)},
        };
 
        int idx = min((int)bosses.size()-1, (floor/5)-1);
        auto& [n,t,s,xp,gld] = bosses[idx];
        return make_unique<BossEnemy>(n,t,s,floor,xp,gld);
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  BATTLE SYSTEM
// ═══════════════════════════════════════════════════════════════
struct BattleResult {
    enum class Outcome { VICTORY, DEFEAT, ESCAPED, SAVE_EXIT };
    Outcome outcome;
    int xpGained=0, goldGained=0;
};
 
class Battle {
    Character& player;
    Enemy&     enemy;
    int        turn=0;
 
    // Critical hit calculation using AGI + LUCK
    pair<int,bool> calcDmg(int baseDmg, int agi, int luck){
        int critPct = min(55, agi/3 + luck/2);
        bool crit   = randInt(1,100)<=critPct;
        float mult  = crit ? (1.5f + luck*0.01f) : 1.0f;
        int   dmg   = max(1, (int)(baseDmg*mult) + randInt(-baseDmg/6, baseDmg/6));
        return {dmg, crit};
    }
 
 
    void drawHUD(){
        clearScr();
        const size_t SW = 14; // sprite column width
 
        // ── Title bar ──
        printSep("═",70, Color::GRAY);
        print("  "+player.getName()+" ["+player.getClassName()+"]", player.getClassColor());
        print("   T"+to_string(turn)+" Fl"+to_string(player.getFloor()), Color::GRAY);
        print("    vs    ", Color::YELLOW);
        println(enemy.getName()+" [Lv"+to_string(enemy.getLevel())+"]", enemy.getColor());
        printSep("═",70, Color::GRAY);
 
        // ── Face-off sprite rows ──
        auto pSp = getBattleSprite(player.getClassType());
        auto eSp = getEnemySprite(enemy.getName());
 
        // Boss extra HP bar above sprites
        if(enemy.isBoss()){
            auto* b = dynamic_cast<BossEnemy*>(&enemy);
            if(b) b->displayStatusBoss();
        }
 
        // Centre column text (7 rows)
        vector<pair<string,Color>> mid={
            {"   *** VS ***    ", Color::YELLOW},
            {"   >========<   ", Color::RED   },
            {"   ~  CLASH  ~  ", Color::YELLOW},
            {"                ", Color::WHITE },
            {"                ", Color::WHITE },
            {"                ", Color::WHITE },
            {"                ", Color::WHITE },
        };
 
        size_t rows = max({pSp.size(), eSp.size(), mid.size()});
        for(size_t r=0; r<rows; r++){
            // Player sprite col
            if(r < pSp.size()){
                setColor(pSp[r].second);
                cout << "  " << padRight(pSp[r].first, SW);
                resetColor();
            } else { cout << string(SW+2,' '); }
 
            // Centre
            if(r < mid.size()){
                setColor(mid[r].second); cout << mid[r].first; resetColor();
            } else { cout << string(16,' '); }
 
            // Enemy sprite col
            if(r < eSp.size()){
                setColor(eSp[r].second);
                cout << padRight(eSp[r].first, SW);
                resetColor();
            }
            cout << "\n";
        }
        println("", Color::WHITE);
 
        // ── HP bars side-by-side ──
        auto& ps  = player.getStats();
        auto& es  = enemy.getStats();
        const int bLen = 17;
        const int gap  = 3;
 
        Color phc = ps.hp>ps.maxHp*0.5f?Color::GREEN:ps.hp>ps.maxHp*0.2f?Color::YELLOW:Color::RED;
        Color ehc = es.hp>es.maxHp*0.5f?Color::GREEN:es.hp>es.maxHp*0.2f?Color::YELLOW:Color::RED;
 
        // Row: player HP  |  enemy HP
        print("  HP[", Color::WHITE);
        drawBar(ps.hp,ps.maxHp,bLen,phc,Color::GRAY);
        {
            string s="]"+to_string(ps.hp)+"/"+to_string(ps.maxHp);
            print(s,Color::WHITE);
            int p=gap+(int)(SW-s.size()+2); if(p<2)p=2;
            cout<<string(p,' ');
        }
        print("HP[",Color::WHITE);
        drawBar(es.hp,es.maxHp,bLen,ehc,Color::GRAY);
        println("]"+to_string(es.hp)+"/"+to_string(es.maxHp),Color::WHITE);
 
        // Row: player MP  |  enemy status
        print("  MP[",Color::BLUE);
        drawBar(ps.mp,ps.maxMp,bLen,Color::BLUE,Color::GRAY);
        {
            string s="]"+to_string(ps.mp)+"/"+to_string(ps.maxMp);
            print(s,Color::BLUE);
            int p=gap+(int)(SW-s.size()+2); if(p<2)p=2;
            cout<<string(p,' ');
        }
        if(enemy.getStatus()!=StatusEffect::NONE)
            print("["+statusName(enemy.getStatus())+":"+to_string(enemy.getStatusTurns())+"t]",
                  statusColor(enemy.getStatus()));
        println("",Color::WHITE);
 
        // Row: player status / XP  |  gold
        if(player.getStatus()!=StatusEffect::NONE)
            print("  ["+statusName(player.getStatus())+":"+to_string(player.getStatusTurns())+"t] ",
                  statusColor(player.getStatus()));
        else
            print("  XP:"+to_string(player.getXp())+"/"+to_string(player.getXpNext())+" ",Color::YELLOW);
        println("Gold:"+to_string(player.getGold())+"g",Color::YELLOW);
 
        printSep("-",70,Color::GRAY);
    }
 
 
public:
    Battle(Character& p, Enemy& e) : player(p), enemy(e){}
 
    BattleResult run(){
        BattleResult result;
 
        while(player.isAlive() && enemy.isAlive()){
            turn++;
            drawHUD();
            println("  Turn "+to_string(turn), Color::GRAY);
            println("", Color::WHITE);
 
            // ── Player status tick ──
            {
                StatusEffect prev = player.getStatus();
                int sd = player.tickStatus();
                if(sd>0){
                    println("  🔥 "+statusName(prev)+" burns you for "
                            +to_string(sd)+" damage!", statusColor(prev));
                    pause_ms(600);
                    if(!player.isAlive()) break;
                    drawHUD();
                }
                if(player.isStunned()){
                    println("  ⚡ You are STUNNED — cannot act this turn!", Color::YELLOW);
                    pause_ms(1000);
                    goto ENEMY_TURN;
                }
            }
 
            // ── Player action ──
            {
                bool acted=false;
                while(!acted){
                    drawHUD();
                    println("  Choose action:", Color::CYAN);
                    println("  1. Attack", Color::WHITE);
                    println("  2. Use Skill", Color::BLUE);
                    println("  3. Use Item", Color::GREEN);
                    println("  4. Flee (AGI-based chance)", Color::GRAY);
                    println("  5. Save & Exit to menu", Color::YELLOW);
                    print(  "  > ", Color::YELLOW);
 
                    int ch; cin >> ch;
                    if(cin.fail()){ safeFlush(); continue; }
 
                    try{
                        switch(ch){
                        case 1:{ // Normal Attack
                            Stats eff = player.effectiveStats();
                            auto [dmg,crit] = calcDmg(eff.str, eff.agi, eff.luck);
                            enemy.getStats().hp -= dmg;
                            enemy.getStats().clamp();
                            if(crit) println("\n  ✨ CRITICAL HIT! Dealt "+to_string(dmg)+" damage!",Color::YELLOW);
                            else     println("\n  ⚔ You strike for "+to_string(dmg)+" damage!",Color::WHITE);
                            acted=true;
                            break;
                        }
                        case 2:{ // Skill
                            auto& sks = player.getSkills();
                            vector<Skill*> avail;
                            for(auto& sk:sks)
                                if(player.getLevel()>=sk->getLvlReq()) avail.push_back(sk.get());
 
                            if(avail.empty()){
                                println("  No skills available yet!", Color::RED);
                                pause_ms(700); break;
                            }
                            println("\n  ── SKILLS ──", Color::CYAN);
                            for(int i=0;i<(int)avail.size();i++){
                                print("  "+to_string(i+1)+". ", Color::GRAY);
                                avail[i]->display();
                            }
                            println("  0. Back", Color::GRAY);
                            print("  > ", Color::YELLOW);
                            int sk; cin>>sk;
                            if(cin.fail()||sk<1||sk>(int)avail.size()){
                                safeFlush(); break;
                            }
                            auto* skill = avail[sk-1];
                            if(player.getStats().mp < skill->getMpCost())
                                throw InsufficientMP();
 
                            player.getStats().mp -= skill->getMpCost();
                            Stats eff = player.effectiveStats();
                            SkillEffect fx = skill->calc(eff);
 
                            if(fx.damage>0){
                                auto [dmg,crit] = calcDmg(fx.damage, eff.agi, eff.luck);
                                enemy.getStats().hp -= dmg;
                                enemy.getStats().clamp();
                                if(crit) println("\n  ✨ ["+skill->getName()+"] CRITICAL! "
                                                 +to_string(dmg)+" damage!", Color::YELLOW);
                                else     println("\n  ✦ ["+skill->getName()+"] "
                                                 +to_string(dmg)+" damage!", Color::CYAN);
                                if(fx.status!=StatusEffect::NONE &&
                                   randInt(1,100)<=fx.statusChance){
                                    enemy.applyStatus(fx.status,3);
                                    println("  → "+enemy.getName()+" is "+
                                            statusName(fx.status)+"!", statusColor(fx.status));
                                }
                            }
                            if(fx.heal>0){
                                int prev=player.getStats().hp;
                                player.getStats().hp=min(player.getStats().maxHp,
                                                         player.getStats().hp+fx.heal);
                                int got=player.getStats().hp-prev;
                                println("\n  ✦ ["+skill->getName()+"] Healed "
                                        +to_string(got)+" HP!", Color::GREEN);
                                if(fx.status==StatusEffect::BLESSED)
                                    player.applyStatus(StatusEffect::BLESSED,3);
                            }
                            acted=true;
                            break;
                        }
                        case 3:{ // Item
                            auto& inv = player.getInv();
                            vector<pair<int,Item*>> usable;
                            for(int i=0;i<(int)inv.size();i++)
                                if(inv[i]->getType()==ItemType::POTION)
                                    usable.push_back({i,inv[i].get()});
                            if(usable.empty()){
                                println("  No usable items!", Color::RED);
                                pause_ms(700); break;
                            }
                            println("\n  ── ITEMS ──", Color::GREEN);
                            for(int i=0;i<(int)usable.size();i++){
                                print("  "+to_string(i+1)+". ", Color::GRAY);
                                usable[i].second->display();
                            }
                            println("  0. Back", Color::GRAY);
                            print("  > ", Color::YELLOW);
                            int it; cin>>it;
                            if(cin.fail()||it<1||it>(int)usable.size()){
                                safeFlush(); break;
                            }
                            string res = player.useItem(usable[it-1].first);
                            println("  "+res, Color::GREEN);
                            acted=true;
                            break;
                        }
                        case 4:{ // Flee
                            Stats eff = player.effectiveStats();
                            int pct = min(78, 35 + eff.agi*2);
                            if(randInt(1,100)<=pct){
                                println("\n  You flee successfully!", Color::YELLOW);
                                pause_ms(800);
                                result.outcome = BattleResult::Outcome::ESCAPED;
                                return result;
                            } else {
                                println("\n  Failed to flee!", Color::RED);
                                acted=true;
                            }
                            break;
                        }
                        case 5:{ // Save & Exit
                            println("\n  Save and exit to main menu? (y/n)", Color::YELLOW);
                            print("  > ", Color::YELLOW);
                            char c; cin>>c; safeFlush();
                            if(tolower(c)=='y'){
                                result.outcome = BattleResult::Outcome::SAVE_EXIT;
                                return result;
                            }
                            break;
                        }
                        default:
                            println("  Invalid choice.", Color::RED);
                            pause_ms(500);
                        }
                    } catch(const GameException& e){
                        println("  ⚠ "+string(e.what()), Color::RED);
                        pause_ms(800);
                    }
                }
            }
 
            if(!enemy.isAlive()) break;
            pause_ms(500);
 
            ENEMY_TURN:
            // ── Enemy status tick ──
            {
                StatusEffect prev = enemy.getStatus();
                int sd = enemy.tickStatus();
                if(sd>0){
                    println("  🔥 "+statusName(prev)+" damages "+enemy.getName()
                            +" for "+to_string(sd)+"!", statusColor(prev));
                    pause_ms(500);
                    if(!enemy.isAlive()) break;
                }
                if(enemy.isStunned()){
                    println("  "+enemy.getName()+" is STUNNED!", Color::YELLOW);
                    pause_ms(800);
                    continue;
                }
            }
 
            {
                int dmg = enemy.attack(player.getStats());
                println("\n  💀 "+enemy.getName()+" attacks you for "
                        +to_string(dmg)+" damage!", Color::RED);
                pause_ms(600);
            }
        }
 
        if(player.isAlive() && !enemy.isAlive()){
            result.outcome   = BattleResult::Outcome::VICTORY;
            result.xpGained  = enemy.getXpReward();
            result.goldGained= enemy.getGldReward();
        } else {
            result.outcome   = BattleResult::Outcome::DEFEAT;
        }
        return result;
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  SHOP
// ═══════════════════════════════════════════════════════════════
class Shop {
    vector<shared_ptr<Item>> stock;
public:
    Shop(int floor){
        string tier = floor<=5 ? "low" : floor<=12 ? "mid" : "high";
        stock.push_back(ItemFactory::createWeapon(tier));
        stock.push_back(ItemFactory::createArmor(tier));
        stock.push_back(ItemFactory::createPotion(false));
        stock.push_back(ItemFactory::createPotion(false));
        stock.push_back(ItemFactory::createMpPotion());
        if(floor>6) stock.push_back(ItemFactory::createPotion(true));
    }
 
    void open(Character& player){
        while(true){
            clearScr();
            displayBox("⚗  THE WANDERING MERCHANT  ⚗", Color::YELLOW);
            println("  Gold: "+to_string(player.getGold())+"g", Color::YELLOW);
            printSep("-",44,Color::GRAY);
            println("  Stock:", Color::CYAN);
            for(int i=0;i<(int)stock.size();i++){
                print("  "+to_string(i+1)+". ", Color::GRAY);
                stock[i]->display();
                println("      Price: "+to_string(stock[i]->getValue())+"g", Color::YELLOW);
            }
            println("\n  0. Leave shop", Color::GRAY);
            print("  > ", Color::YELLOW);
            int ch; cin>>ch;
            if(cin.fail()||ch==0){ safeFlush(); break; }
            if(ch<1||ch>(int)stock.size()){ safeFlush(); continue; }
 
            try{
                auto& item = stock[ch-1];
                player.spendGold(item->getValue());
                player.addItem(item);
                println("  Purchased "+item->getName()+"!", Color::GREEN);
                stock.erase(stock.begin()+ch-1);
                pause_ms(700);
                if(stock.empty()){ println("  Merchant is sold out!", Color::GRAY); pause_ms(800); break; }
            } catch(const GameException& e){
                println("  ⚠ "+string(e.what()), Color::RED);
                pause_ms(800);
            }
        }
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  SAVE / LOAD SYSTEM  (File I/O)
// ═══════════════════════════════════════════════════════════════
class SaveSystem {
    static const string FILE;
public:
    static void save(const Character& ch){
        ofstream f(FILE);
        if(!f) throw SaveException("Cannot write save file");
        f << ch.serialize();
        println("  💾 Game saved.", Color::GREEN);
    }
 
    static bool hasSave(){
        return ifstream(FILE).good();
    }
 
    // Returns {name, classType(int), level}
    static tuple<string,int,int> peekSave(){
        ifstream f(FILE);
        if(!f) return {"",0,0};
        string name; getline(f,name);
        int ct,lv,xp,xpN,gld,fl;
        f >> ct >> lv >> xp >> xpN >> gld >> fl;
        return {name, ct, lv};
    }
 
    static void load(Character& ch){
        ifstream f(FILE);
        if(!f) throw SaveException("No save file found");
        string name; getline(f,name);
        ch.setName(name);
        int ct; f >> ct; // already read by caller for class selection
        ch.deserializeBody(f);
        ch.initSkills();
    }
 
    static void deleteSave(){ remove(FILE.c_str()); }
};
const string SaveSystem::FILE = "chronicles_save.dat";
 
// ═══════════════════════════════════════════════════════════════
//  GAME  (Main Controller)
// ═══════════════════════════════════════════════════════════════
class Game {
    unique_ptr<Character> player;
    int  floor     = 1;
    bool exitToMenu= false;   // set by Save&Exit / Exit-to-menu options
    static const int MAX_FLOOR = 20;
 
    // Factory method: create character by class
    unique_ptr<Character> makeCharacter(int cls, const string& name){
        switch(cls){
            case 1: return make_unique<Warrior>(name);
            case 2: return make_unique<Mage>   (name);
            case 3: return make_unique<Rogue>  (name);
            case 4: return make_unique<Paladin>(name);
            default: throw InvalidInput("Class must be 1-4");
        }
    }
 
    void showTitle(){
        clearScr();
        setColor(Color::CYAN);
        cout <<
R"(
  ██████╗██╗  ██╗██████╗  ██████╗ ███╗   ██╗██╗ ██████╗██╗     ███████╗███████╗
 ██╔════╝██║  ██║██╔══██╗██╔═══██╗████╗  ██║██║██╔════╝██║     ██╔════╝██╔════╝
 ██║     ███████║██████╔╝██║   ██║██╔██╗ ██║██║██║     ██║     █████╗  ███████╗
 ██║     ██╔══██║██╔══██╗██║   ██║██║╚██╗██║██║██║     ██║     ██╔══╝  ╚════██║
 ╚██████╗██║  ██║██║  ██║╚██████╔╝██║ ╚████║██║╚██████╗███████╗███████╗███████║
  ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝╚══════╝╚══════╝╚══════╝
)";
        setColor(Color::MAGENTA);
        cout << "                        ✦  OF THE ABYSS  ✦\n";
        resetColor();
        setColor(Color::GRAY);
        cout << "                      A Pure C++17 Terminal RPG\n";
        resetColor();
        println("", Color::WHITE);
    }
 
    void classSelectScreen(){
        clearScr();
        displayBox("SELECT YOUR CLASS", Color::CYAN);
        println("", Color::WHITE);
        printAllSprites();
        printSep("-",80,Color::GRAY);
        println("", Color::WHITE);
 
        print("  1. ⚔  WARRIOR  ", Color::RED);
        println("─ High HP & STR, tanky frontliner, rage-based abilities",Color::WHITE);
        println("     STR★★★★★  DEF★★★★☆  HP★★★★★  AGI★★☆☆☆  INT★☆☆☆☆\n",Color::GRAY);
 
        print("  2. 🔮  MAGE    ", Color::BLUE);
        println("─ Fragile but devastating, INT-scaled spells & status",Color::WHITE);
        println("     STR★☆☆☆☆  DEF★★☆☆☆  HP★★☆☆☆  AGI★★★☆☆  INT★★★★★\n",Color::GRAY);
 
        print("  3. 🗡  ROGUE   ", Color::DARK_GREEN);
        println("─ AGI & LUCK king, crits constantly, bleeds enemies",Color::WHITE);
        println("     STR★★★★☆  DEF★★☆☆☆  HP★★★☆☆  AGI★★★★★  LCK★★★★★\n",Color::GRAY);
 
        print("  4. 🛡  PALADIN ", Color::YELLOW);
        println("─ Holy warrior, balanced, heals + smites enemies",Color::WHITE);
        println("     STR★★★☆☆  DEF★★★★★  HP★★★★☆  AGI★★☆☆☆  INT★★★☆☆\n",Color::GRAY);
 
        print("  > ", Color::YELLOW);
    }
 
    // Shows a full-screen class preview with large sprite before confirming
    void showClassPreview(int cls){
        ClassType ct;
        string cname, cdesc, cstats;
        Color cc;
        switch(cls){
        case 1: ct=ClassType::WARRIOR; cc=Color::RED;
            cname="WARRIOR"; cdesc="Iron will, unbreakable armour, unbridled fury.";
            cstats="STR:20  DEF:14  AGI:8  INT:4  LCK:6  HP:140  MP:25"; break;
        case 2: ct=ClassType::MAGE;    cc=Color::BLUE;
            cname="MAGE";    cdesc="Knowledge is destruction. Reality bends to your will.";
            cstats="STR:4   DEF:4   AGI:10  INT:24  LCK:8  HP:70  MP:90"; break;
        case 3: ct=ClassType::ROGUE;   cc=Color::DARK_GREEN;
            cname="ROGUE";   cdesc="Shadow, speed, silence. Strike before they see you.";
            cstats="STR:16  DEF:7   AGI:22  INT:7  LCK:18  HP:90  MP:55"; break;
        default:ct=ClassType::PALADIN; cc=Color::YELLOW;
            cname="PALADIN"; cdesc="Blessed steel, divine light, unshakeable faith.";
            cstats="STR:13  DEF:17  AGI:8   INT:14  LCK:10  HP:110  MP:65"; break;
        }
        clearScr();
        printSep("═",62,cc);
        print("        YOU CHOSE: ", cc);
        println(cname, cc);
        printSep("═",62,cc);
        println("", Color::WHITE);
        printSprite(ct, "          ");
        println("", Color::WHITE);
        println("  \""+cdesc+"\"", Color::GRAY);
        println("", Color::WHITE);
        println("  Base Stats:  "+cstats, Color::CYAN);
        println("", Color::WHITE);
        printSep("-",62,Color::GRAY);
        println("  Press ENTER to confirm, or 0 to go back.", Color::GRAY);
        print("  > ", Color::YELLOW);
    }
 
    void statDistScreen(){
        while(player->getStats().statPts > 0){
            clearScr();
            displayBox("DISTRIBUTE STAT POINTS", Color::CYAN);
            player->displayStats();
            println("\n  You have "+to_string(player->getStats().statPts)
                    +" point(s) to allocate.", Color::YELLOW);
            println("  Stats: STR | DEF | AGI | INT | LCK | HP | MP",Color::CYAN);
            println("  Example:  STR 3   (adds 3 to STR)",Color::GRAY);
            print("  > ", Color::YELLOW);
 
            string stat; int pts;
            try{
                if(!(cin >> stat >> pts)){
                    safeFlush();
                    throw InvalidInput("Enter stat name and number");
                }
                player->distributeStat(stat, pts);
                println("  ✓ Added "+to_string(pts)+" to "+stat+"!", Color::GREEN);
                pause_ms(400);
            } catch(const GameException& e){
                safeFlush();
                println("  ⚠ "+string(e.what()), Color::RED);
                pause_ms(800);
            }
        }
    }
 
    void showSkills(){
        clearScr();
        printSprite(player->getClassType(),"    ");
        displayBox(player->getName()+" Skills", player->getClassColor());
        println("  Available at your current level ("+to_string(player->getLevel())+"):\n", Color::GRAY);
        bool any=false;
        for(auto& sk : player->getSkills()){
            bool unlocked = player->getLevel() >= sk->getLvlReq();
            if(unlocked){ sk->display(); any=true; }
        }
        if(!any) println("  No skills unlocked yet!", Color::GRAY);
        println("\n  Locked (future):", Color::GRAY);
        for(auto& sk : player->getSkills())
            if(player->getLevel() < sk->getLvlReq()){ sk->display(); }
        pressEnter();
    }
 
    // Returns true if the game loop should break (exit/quit)
    bool showGameMenu(){
        while(true){
            clearScr();
            // Mini sprite + name in menu header
            auto sprite = getClassSprite(player->getClassType());
            for(size_t i=0;i<sprite.size();i++){
                if(i==2){
                    setColor(sprite[i].second);
                    cout << "  " << sprite[i].first;
                    resetColor();
                    print("   ╔═══════════════════════╗\n", Color::CYAN);
                } else if(i==3){
                    setColor(sprite[i].second);
                    cout << "  " << sprite[i].first;
                    resetColor();
                    print("   ║    ─ GAME MENU ─      ║\n", Color::CYAN);
                } else if(i>=4 && i<=8){
                    static const vector<string> opts={
                        "║  1. Resume              ║",
                        "║  2. View Stats          ║",
                        "║  3. View Inventory      ║",
                        "║  4. View Skills         ║",
                        "║  5. Distribute Stats    ║",
                    };
                    size_t oi = i-4;
                    setColor(sprite[i].second);
                    cout << "  " << sprite[i].first;
                    resetColor();
                    if(oi < opts.size()){
                        print("   ", Color::WHITE);
                        println(opts[oi], Color::CYAN);
                    } else cout << "\n";
                } else if(i==9){
                    setColor(sprite[i].second);
                    cout << "  " << sprite[i].first;
                    resetColor();
                    print("   ╚═══════════════════════╝\n", Color::CYAN);
                } else {
                    setColor(sprite[i].second);
                    cout << "  " << sprite[i].first << "\n";
                    resetColor();
                }
            }
 
            println("", Color::WHITE);
            printSep("-",44,Color::GRAY);
            println("  6. Save Game",          Color::GREEN);
            println("  7. Save and Exit",      Color::YELLOW);
            println("  8. Exit to Main Menu",  Color::RED);
            println("  9. Quit Game",          Color::DARK_RED);
            printSep("-",44,Color::GRAY);
            print("  > ", Color::YELLOW);
 
            int ch; cin>>ch;
            if(cin.fail()){ safeFlush(); continue; }
 
            switch(ch){
            case 1:
                return false;   // resume
            case 2:
                clearScr();
                player->displayStats();
                pressEnter();
                break;
            case 3:
                clearScr();
                player->displayInventory();
                println("\n  Equip/use an item? Enter slot # or 0 to back:", Color::CYAN);
                print("  > ", Color::YELLOW);
                {
                    int slot; cin>>slot;
                    if(!cin.fail() && slot>0){
                        try{
                            string r = player->useItem(slot-1);
                            println("  "+r, Color::GREEN);
                            pause_ms(700);
                        } catch(const GameException& e){
                            println("  "+string(e.what()), Color::RED);
                            pause_ms(700);
                        }
                    } else safeFlush();
                }
                break;
            case 4:
                showSkills();
                break;
            case 5:
                statDistScreen();
                break;
            case 6:
                try{
                    SaveSystem::save(*player);
                    println("  Saved!", Color::GREEN);
                    pause_ms(700);
                } catch(const SaveException& e){
                    println("  "+string(e.what()), Color::RED);
                    pause_ms(800);
                }
                break;
            case 7:
                try{ SaveSystem::save(*player); }
                catch(...){ println("  Warning: save failed.", Color::RED); pause_ms(600); }
                println("  Saving and returning to main menu...", Color::YELLOW);
                pause_ms(800);
                exitToMenu = true;
                return false;
            case 8:
                println("  Return to main menu WITHOUT saving?", Color::RED);
                println("  Progress since last auto-save will be lost. (y/n)", Color::YELLOW);
                print("  > ", Color::YELLOW);
                {
                    char c; cin>>c; safeFlush();
                    if(tolower(c)=='y'){
                        exitToMenu = true;
                        return false;
                    }
                }
                break;
            case 9:
                println("  Quit the game? (y/n)", Color::RED);
                print("  > ", Color::YELLOW);
                {
                    char c; cin>>c; safeFlush();
                    if(tolower(c)=='y') return true;   // signal full quit
                }
                break;
            default:
                break;
            }
        }
    }
 
    void doBattle(bool isBoss){
        unique_ptr<Enemy> enemy;
        if(isBoss){
            enemy = EnemyFactory::createBoss(floor);
            auto* boss = dynamic_cast<BossEnemy*>(enemy.get());
            clearScr();
            printSep("░",62, Color::MAGENTA);
            println("  💀  BOSS BATTLE  💀", Color::MAGENTA);
            println("  "+boss->getTitle(), Color::WHITE);
            println("  ☠  "+boss->getName()+"  ☠", Color::RED);
            printSep("░",62, Color::MAGENTA);
            pause_ms(2200);
        } else {
            enemy = EnemyFactory::createRandom(floor);
        }
 
        Battle battle(*player, *enemy);
        BattleResult res = battle.run();
 
        clearScr();
 
        if(res.outcome == BattleResult::Outcome::VICTORY){
            println("", Color::WHITE);
            println("  ╔══════════════════════════════╗", Color::GREEN);
            println("  ║        VICTORY!   🏆         ║", Color::GREEN);
            println("  ╚══════════════════════════════╝", Color::GREEN);
            println("  XP  +"+to_string(res.xpGained),  Color::YELLOW);
            println("  Gold+"+to_string(res.goldGained)+"g", Color::YELLOW);
 
            
            bool lvdUp = player->gainXp(res.xpGained);
            player->addGold(res.goldGained);
 
            if(lvdUp){
                println("\n  🎉 LEVEL UP!  You are now Level "
                        +to_string(player->getLevel())+"!", Color::MAGENTA);
            }
 
            // Random loot drop (STL algorithm: sort inventory to show new item)
            int dropChance = isBoss ? 100 : 38;
            if(randInt(1,100)<=dropChance){
                auto loot = (randInt(1,2)==1)
                    ? ItemFactory::createWeapon(floor<=7?"low":floor<=14?"mid":"high")
                    : ItemFactory::createArmor (floor<=7?"low":floor<=14?"mid":"high");
                println("\n  💎 LOOT DROP:", Color::YELLOW);
                loot->display();
                println("  Pick up? (y/n): ", Color::CYAN);
                char pick=' '; cin>>pick; safeFlush();
                if(tolower(pick)=='y'){
                    try{
                        player->addItem(loot);
                        println("  Added to inventory!", Color::GREEN);
                    } catch(const InventoryFull& e){
                        println("  "+string(e.what()), Color::RED);
                    }
                }
            }
 
            pressEnter();
            if(player->getStats().statPts>0) statDistScreen();
 
        } else if(res.outcome == BattleResult::Outcome::SAVE_EXIT){
            clearScr();
            println("  Saving your progress...", Color::YELLOW);
            try{ SaveSystem::save(*player); }
            catch(const SaveException& e){
                println("  Warning: "+string(e.what()), Color::RED);
                pause_ms(700);
            }
            println("  Returning to main menu.", Color::GREEN);
            pause_ms(900);
            exitToMenu = true;
        } else if(res.outcome == BattleResult::Outcome::DEFEAT){
            println("  ╔══════════════════════════════╗", Color::RED);
            println("  ║       DEFEATED...   💀        ║", Color::RED);
            println("  ╚══════════════════════════════╝", Color::RED);
            println("  You have fallen in the Abyss.", Color::GRAY);
            pressEnter();
            showGameOver();
            player.reset();
        } else {
            println("  You retreated from battle.", Color::YELLOW);
            pressEnter();
        }
    }
 
    void runFloor(){
        bool bossFloor = (floor%5==0);
        int  encounters= bossFloor ? 2 : 3;
 
        for(int i=0; i<encounters && player && player->isAlive(); i++){
            if(exitToMenu) return;
 
            // Between-encounter menu prompt
            if(i>0){
                clearScr();
                printSprite(player->getClassType(),"    ");
                printSep("-",44,Color::GRAY);
                print("  Floor "+to_string(floor), Color::CYAN);
                println("   Encounter "+to_string(i+1)+"/"+to_string(encounters), Color::WHITE);
                auto& s=player->getStats();
                print("  HP: "+to_string(s.hp)+"/"+to_string(s.maxHp), Color::GREEN);
                println("   MP: "+to_string(s.mp)+"/"+to_string(s.maxMp), Color::BLUE);
                printSep("-",44,Color::GRAY);
                println("  1 = next encounter    M = open menu", Color::GRAY);
                print("  > ", Color::YELLOW);
                safeFlush();
                string inp; getline(cin, inp);
                if(!inp.empty() && (inp[0]=='m'||inp[0]=='M')){
                    bool quit = showGameMenu();
                    if(quit){ player.reset(); return; }
                    if(exitToMenu) return;
                }
            } else {
                clearScr();
                print("  Floor "+to_string(floor), Color::CYAN);
                println("   Encounter "+to_string(i+1)+"/"+to_string(encounters), Color::WHITE);
                pause_ms(900);
            }
            doBattle(false);
        }
        if(!player || !player->isAlive() || exitToMenu) return;
 
        // Shop opportunity
        if(randInt(1,100)<=65 || floor%5==4){
            clearScr();
            println("  🏪  A merchant materialises from the shadows!", Color::YELLOW);
            println("  Enter the shop? (y/n)", Color::CYAN);
            char ch=' '; cin>>ch; safeFlush();
            if(tolower(ch)=='y'){
                Shop shop(floor);
                shop.open(*player);
            }
        }
 
        // Regen between floors
        auto& s = player->getStats();
        int hR = (int)(s.maxHp*0.25f), mR = (int)(s.maxMp*0.40f);
        s.hp = min(s.maxHp, s.hp+hR);
        s.mp = min(s.maxMp, s.mp+mR);
        clearScr();
        println("  Resting...  +"+to_string(hR)+" HP  +"+to_string(mR)+" MP", Color::GREEN);
        pause_ms(1100);
        if(exitToMenu) return;
 
        // Boss fight on multiples of 5
        if(bossFloor && player && player->isAlive())
            doBattle(true);
 
        if(exitToMenu) return;
        if(player && player->isAlive())
            SaveSystem::save(*player);
    }
 
    void showGameOver(){
        clearScr();
        setColor(Color::RED);
        cout <<
R"(
   ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗
  ██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗
  ██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝
  ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗
  ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║
   ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝
)";
        resetColor();
        if(player){
            println("  Floor reached : "+to_string(player->getFloor()), Color::YELLOW);
            println("  Level         : "+to_string(player->getLevel()), Color::YELLOW);
            println("  Gold           : "+to_string(player->getGold())+"g", Color::YELLOW);
        }
        SaveSystem::deleteSave();
        pressEnter();
    }
 
    void showVictory(){
        clearScr();
        setColor(Color::YELLOW);
        cout <<
R"(
 ██╗   ██╗██╗ ██████╗████████╗ ██████╗ ██████╗ ██╗   ██╗██╗
 ██║   ██║██║██╔════╝╚══██╔══╝██╔═══██╗██╔══██╗╚██╗ ██╔╝██║
 ██║   ██║██║██║        ██║   ██║   ██║██████╔╝ ╚████╔╝ ██║
 ╚██╗ ██╔╝██║██║        ██║   ██║   ██║██╔══██╗  ╚██╔╝  ╚═╝
  ╚████╔╝ ██║╚██████╗   ██║   ╚██████╔╝██║  ██║   ██║   ██╗
   ╚═══╝  ╚═╝ ╚═════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝
)";
        resetColor();
        println("  🏆  You have conquered the Abyss and claimed godhood!  🏆", Color::CYAN);
        if(player){
            println("  "+player->getName()+" | "+player->getClassName()
                    +" | Level "+to_string(player->getLevel()), Color::WHITE);
            println("  Gold hoarded: "+to_string(player->getGold())+"g", Color::YELLOW);
        }
        SaveSystem::deleteSave();
        pressEnter();
    }
 
public:
    void run(){
        while(true){
            showTitle();
            printSep("═");
            println("  1. New Game", Color::WHITE);
            if(SaveSystem::hasSave()){
                auto [nm,ct,lv] = SaveSystem::peekSave();
                println("  2. Continue  ("+nm+" ─ Level "+to_string(lv)+")", Color::GREEN);
            }
            println("  3. View Controls", Color::GRAY);
            println("  4. Quit", Color::GRAY);
            printSep("═");
            print("  > ", Color::YELLOW);
 
            int ch; cin>>ch;
            if(cin.fail()){ safeFlush(); continue; }
 
            if(ch==1){
                // ─── New Game ───
                int cls = 0;
                while(cls<1||cls>4){
                    classSelectScreen();
                    cin>>cls;
                    if(cin.fail()){ safeFlush(); cls=0; continue; }
                    if(cls<1||cls>4){ safeFlush(); continue; }
                    // Show preview and let player confirm or go back
                    showClassPreview(cls);
                    string conf; cin.ignore(); getline(cin,conf);
                    if(!conf.empty() && conf[0]=='0'){ cls=0; }
                }
 
                clearScr();
                println("  Enter your hero's name:", Color::CYAN);
                print("  > ", Color::YELLOW);
                string nm; cin>>nm; safeFlush();
 
                try{ player = makeCharacter(cls, nm); }
                catch(...){ player = make_unique<Warrior>(nm); }
 
                // Observer: announce level-up events
                player->addListener([&](const string& ev){
                    if(ev=="LEVEL_UP")
                        println("\n  ⬆ LEVEL UP! New level: "
                                +to_string(player->getLevel()), Color::MAGENTA);
                });
 
                clearScr();
                println("  📜  PROLOGUE", Color::YELLOW);
                printSep("═");
                println("  The Abyss — a dungeon twenty floors deep, older than memory.", Color::WHITE);
                println("  Those who reach the bottom are said to transcend mortality.", Color::WHITE);
                println("  You, "+nm+" the "+player->getClassName()+", step into the darkness...\n",Color::CYAN);
                player->getStats().statPts = 5;
                pressEnter();
                statDistScreen();
                floor=1; player->setFloor(floor);
 
            } else if(ch==2 && SaveSystem::hasSave()){
                // ─── Load Game ───
                auto [nm,ct,lv] = SaveSystem::peekSave();
                try{
                    player = makeCharacter(ct+1, nm);
                    SaveSystem::load(*player);
                    floor = player->getFloor();
                    println("  Welcome back, "+player->getName()+"!", Color::GREEN);
                    pause_ms(1000);
                } catch(const GameException& e){
                    println("  ⚠ "+string(e.what()), Color::RED);
                    pause_ms(1000); continue;
                }
 
            } else if(ch==3){
                clearScr();
                displayBox("CONTROLS & TIPS", Color::CYAN);
                println("  Battle:  1=Attack  2=Skill  3=Item  4=Flee", Color::WHITE);
                println("  Stats:   STR=Strength  DEF=Defence  AGI=Agility",Color::WHITE);
                println("           INT=Intelligence  LCK=Luck  HP  MP",Color::WHITE);
                println("  Classes: Warrior(1) Mage(2) Rogue(3) Paladin(4)",Color::WHITE);
                println("  Floors 5,10,15,20 are BOSS floors.", Color::YELLOW);
                println("  25% HP and 40% MP restored between floors.",Color::GREEN);
                println("  Game is auto-saved after every floor cleared.",Color::GRAY);
                pressEnter(); continue;
 
            } else if(ch==4){
                println("  Farewell, brave soul.", Color::GRAY);
                break;
            } else {
                continue;
            }
 
            // ─── Main Dungeon Loop ───
            exitToMenu = false;
            while(player && player->isAlive() && floor<=MAX_FLOOR && !exitToMenu){
                clearScr();
                print("  Descending to Floor "+to_string(floor), Color::CYAN);
                println(floor%5==0 ? "   [BOSS FLOOR]" : "", Color::MAGENTA);
                pause_ms(1100);
 
                player->setFloor(floor);
                runFloor();
 
                if(!player || !player->isAlive() || exitToMenu) break;
                floor++;
            }
 
            if(exitToMenu){
                exitToMenu = false;
                // Return to main menu — player state preserved for continue
                continue;
            }
 
            if(player && player->isAlive() && floor>MAX_FLOOR){
                showVictory();
                player.reset();
            }
        }
    }
};
 
// ═══════════════════════════════════════════════════════════════
//  ENTRY POINT
// ═══════════════════════════════════════════════════════════════
int main(){
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
 
    try{
        Game game;
        game.run();
    } catch(const exception& e){
        cerr << "\n[FATAL] " << e.what() << endl;
        return 1;
    } catch(...){
        cerr << "\n[FATAL] Unknown error!" << endl;
        return 1;
    }
    return 0;
}
 