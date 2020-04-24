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
    Disable,  // 未配置Reader
    Title,    // 稱號
};
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

class READER
{
public:
    PassMode mode = Wakaka;
    int countKeys;
    int passKeys[];
    int specificKey;
    String modeName[8] = {"[Counting]", "[Specify]", "[OR]", "[And]", "[Wakaka]", "[Special]", "[Disable]", "[Title]"};

    READER() {}

    void SetKeyCount(int count)
    {
        mode = Counting;
        countKeys = count;
        // Serial.println(F(" *   Mode   ==> Counting"));
        Show();
    }
    void SetSpecificKey(int key)
    {
        mode = Specify;
        specificKey = key;
        // Serial.println(F(" *   Mode   ==> Specific Key"));
        Show();
    }
    // 2020/04/20 新方法
    void AddPassKey(int key, int index)
    {
        passKeys[index] = key;
    }
    void SetMode(PassMode useMode, int count)
    {
        mode = useMode;
        countKeys = count;
        // Serial.println(mode);
        Show();
    }
    void SetTitle(byte *title, int name)
    {
        mode = Title;
        String character;
        switch (name)
        {
        case 1:
            character = "Arthur";
            break;
        case 2:
            character = "Merlin";
            break;
        case 3:
            character = "Lancelot";
            break;
        case 4:
            character = "Galahad";
            break;
        case 5:
            character = "Percival";
            break;
        case 6:
            character = "Bors";
            break;
        case 7:
            character = "Guinevere";
            break;
        case 8:
            character = "Excalibur";
            break;
        case 9:
            character = "SwordStone";
            break;
        case 10:
            character = "Viviane";
            break;
        default:
            break;
        }
        character.getBytes(title, 18);
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
            Serial.print(F(" *   Target ==> [Ruby"));
            if (specificKey < 10)
            {
                Serial.print(F("0"));
                Serial.print(specificKey);
            }
            else
                Serial.print(specificKey);
            Serial.println(F("]"));
            break;
        case OR:
            for (int i = 0; i < countKeys; i++)
            {
                Serial.print(F(" *   Target ==> [Ruby"));
                if (passKeys[i] < 10)
                {
                    Serial.print(F("0"));
                    Serial.print(passKeys[i]);
                }
                else
                    Serial.print(passKeys[i]);
                Serial.println(F("]"));
            }
            break;
        case AND:
            for (int i = 0; i < countKeys; i++)
            {
                Serial.print(F(" *   Target ==> [Ruby"));
                if (passKeys[i] < 10)
                {
                    Serial.print(F("0"));
                    Serial.print(passKeys[i]);
                }
                else
                    Serial.print(passKeys[i]);
                Serial.println(F("]"));
            }
            break;
        case Wakaka:
            // Serial.println();
            break;
        case Special:
            Serial.print(F(" *   Target ==> [Ruby"));
            if (specificKey < 10)
            {
                Serial.print(F("0"));
                Serial.print(specificKey);
            }
            else
                Serial.print(specificKey);
            Serial.println(F("]"));
            break;
        default:
            break;
        }
    }

private:
};