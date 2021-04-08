#define READER_h
#define stringify(name) #name
enum PassMode
{
    Counting = 0, // 計數密鑰 Pass ==> Type C
    Specify = 1,  // 指定密鑰 Pass ==> Type V
    OR = 2,       // 擇一密鑰 Pass ==> Type R
    AND = 3,      // 多重密鑰 Pass ==> Type A
    Wakaka = 4,   // 萬用密鑰 Pass  ==> Type W
    Special = 5,  // 針對單一Reader額外配置 ==> Type X
    Disable = 6,  // 未配置Reader
    Title = 7,    // 稱號
};
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

class READER
{
public:
    PassMode mode = Wakaka;
    int countKeys = 0;
    bool passKeys[11];
    int specificKey;
    // String modeName[8]; // 刪掉就當機，原因不明
    char const *Pass(PassMode value)
    {
        static char const *table[] = {"[Counting]", "[Specify]", "[OR]", "[And]", "[Wakaka]", "[Special]", "[Disable]", "[Title]"};
        return table[value];
    }

    READER() {}

    void SetKeyCount(int count)
    {
        mode = Counting;
        countKeys = count;
        Show();
    }
    // void SetSpecificKey(int key)
    // {
    //     mode = Specify;
    //     specificKey = key;
    //     Show();
    // }
    // void SetSpecificKey(PassMode useMode, int key)
    // {
    //     mode = useMode;
    //     specificKey = key;
    //     Show();
    // }
    // 2020/06/30 新方法
    void AddPassKey(int key)
    {
        countKeys++;
        specificKey = key;
        passKeys[key] = true;
    }
    void SetMode(PassMode useMode)
    {
        mode = useMode;
        Show();
    }
    // 2020/04/20 新方法
    // void AddPassKey(int key, int index)
    // {
    //     passKeys[index] = key;
    // }
    // void SetMode(PassMode useMode, int count)
    // {
    //     mode = useMode;
    //     countKeys = count;
    //     Show();
    // }
    byte *SetTitle(int name)
    {
        mode = Title;
        byte *title;
        switch (name)
        {
        case 1:
            title = (byte *)"Arthur";
            break;
        case 2:
            title = (byte *)"Merlin";
            break;
        case 3:
            title = (byte *)"Lancelot";
            break;
        case 4:
            title = (byte *)"Galahad";
            break;
        case 5:
            title = (byte *)"Percival";
            break;
        case 6:
            title = (byte *)"Bors";
            break;
        case 7:
            title = (byte *)"Guinevere";
            break;
        case 8:
            title = (byte *)"Excalibur";
            break;
        case 9:
            title = (byte *)"SwordStone";
            break;
        case 10:
            title = (byte *)"Viviane";
            break;
        default:
            break;
        }
        Show();
        return title;
    }

    void Show()
    {
        Serial.print(F(" *   Mode   ==> "));
        Serial.println(Pass(mode));
        switch (mode)
        {
        case Counting:
            Serial.print(F(" *   Target ==> x"));
            Serial.println(countKeys);
            break;
        case Specify:
        case OR:
        case AND:
        case Special:
            for (int i = 1; i < 11; i++)
            {
                if (passKeys[i])
                {
                    Serial.print(F(" *   Target ==> [Ruby"));
                    if (i < 10)
                        Serial.print(F("0"));
                    Serial.print(i);
                    Serial.println(F("]"));
                }
            }
            break;
        case Wakaka:
            break;
        default:
            break;
        }
    }

private:
};