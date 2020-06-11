#define READER_h
#define stringify(name) #name
enum PassMode
{
    Counting, // 計數密鑰 Pass ==> Type C
    Specify,  // 指定密鑰 Pass ==> Type V
    OR,       // 擇一密鑰 Pass ==> Type R
    AND,      // 多重密鑰 Pass ==> Type A
    Wakaka,   // 萬用密鑰 Pass  ==> Type W
    Special,  // 針對單一Reader額外配置 ==> Type X
};
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

class READER
{
public:
    String modeName[6] = {"[Counting]", "[Specify]", "[OR]", "[And]", "[Wakaka]", "[Special]"};
    PassMode mode = Wakaka;
    int countKeys;
    RFID *specificKey;
    RFID *passKeys[];

    // RFID *rfid;
    READER() {}

    void Initialize(int count)
    {
        mode = Counting;
        countKeys = count;
        Show();
    }

    void Initialize(RFID &key)
    {
        mode = Specify;
        specificKey = &key;
        Show();
    }

    // 2020/04/20 新方法
    void AddPassKey(RFID &key, int index)
    {
        passKeys[index] = &key;
    }

    // OR Mode AND Mode
    template <typename T, size_t N>
    void Initialize(PassMode mode, T (&keys)[N])
    {
        this->mode = mode;
        countKeys = N;
        for (int i = 0; i < countKeys; i++)
        {
            passKeys[i] = keys[i];
        }
        Show();
    }
    void Initialize(PassMode mode, int count)
    {
        this->mode = mode;
        countKeys = count;
        Show();
    }
    void Show()
    {
        Serial.print(F(" *   Mode   ==> "));
        Serial.println(modeName[mode]);
        switch (mode)
        {
        case Counting:
            Serial.print(F(" *   Target ==> x"));
            Serial.println(countKeys);
            break;
        case Specify:
            Serial.print(F(" *   Target ==> "));
            specificKey->ShowRubyName();
            Serial.print(F(" in "));
            specificKey->ShowRubyBlock();
            Serial.println();
            break;
        case OR:
            for (int i = 0; i < countKeys; i++)
            {
                Serial.print(F(" *   Target ==> "));
                passKeys[i]->ShowRubyName();
                Serial.print(F(" in "));
                passKeys[i]->ShowRubyBlock();
                Serial.println();
            }
            break;
        case AND:
            for (int i = 0; i < countKeys; i++)
            {
                Serial.print(F(" *   Target ==> "));
                passKeys[i]->ShowRubyName();
                Serial.print(F(" in "));
                passKeys[i]->ShowRubyBlock();
                Serial.println();
            }
            break;
        case Wakaka:
            // Serial.println();
            break;
        case Special:
            // Serial.println();
            break;
        default:
            break;
        }
    }

private:
};