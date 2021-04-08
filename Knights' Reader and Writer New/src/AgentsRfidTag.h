// 2021-04-08 v2.0 
/****************************************************************************
 * 
 * 1. 寫入內容          RUBY_NUMBER     ASCII       Sector      Block       Knights
 * 		Ruby01			01              49			15		  	00          Arthur
 * 		Ruby02		  	02              50			15		  	01          Merlin
 * 		Ruby03	  		03	    		51			15	  		02          Lancelot
 * 		Ruby04	  		04	    		52			14		  	00          Galahad
 * 		Ruby05	  		05	    		53			14	  		01          Percival
 * 		Ruby06	  		06	    		54			14		  	02          Bors
 * 		Ruby07		  	07	    		55			13		  	00          Guinevere
 * 		Ruby08		  	08	    		56			13		  	01          Excalibur
 * 		Ruby09		  	09	    		57			13		  	02          The Sword in the Stone
 * 		Ruby10		  	10	    		58			12		  	00          Viviane
 * 		Ruby11		  	11	    		59			12		  	01          deleted
 * 		Ruby12		  	12	    		60			12		  	02          deleted
 * 		Ruby13		  	13	    		61			11		  	00          deleted
 * 2. 使用區塊
 * Sector   Block   Content
 * 15       00      Ruby01      Arthur
 * 15       01      Ruby02      Merlin
 * 15       02      Ruby03      Lancelot
 * 14       00      Ruby04      Galahad
 * 14       01      Ruby05      Percival
 * 14       02      Ruby06      Bors
 * 13       00      Ruby07      Guinevere
 * 13       01      Ruby08      Excalibur
 * 13       02      Ruby09      The Sword in the Stone
 * 12       00      Ruby10      Viviane
 * 05       01      #Stage      *unavailable
 * 05       02      #Time       *unavailable
 * 06       00      *Text       Challenge
 * 06       01      *Text       Title
 * 07       00      *Text       ID
 * 07       01      KocmocA     iLYuSha Key
 ****************************************************************************/
// v1.2.200116
/****************************************************************************
 * RFID Writer
 * 
 * 1. 寫入內容          RUBY_NUMBER     ASCII       Sector      Block       Knights
 * 		Wakaka Key		00              48			09~15		00~02       Wakaka
 * 		Ruby01			01              49			15		  	00          Arthur
 * 		Ruby02		  	02              50			15		  	01          Merlin
 * 		Ruby03	  		03	    		51			15	  		02          Lancelot
 * 		Ruby04	  		04	    		52			14		  	00          Galahad
 * 		Ruby05	  		05	    		53			14	  		01          Percival
 * 		Ruby06	  		06	    		54			14		  	02          Bors
 * 		Ruby07		  	07	    		55			13		  	00          Guinevere
 * 		Ruby08		  	08	    		56			13		  	01          Excalibur
 * 		Ruby09		  	09	    		57			13		  	02          The Sword in the Stone
 * 		Ruby10		  	10	    		58			12		  	00          Viviane
 * 		Ruby11		  	11	    		59			12		  	01          deleted
 * 		Ruby12		  	12	    		60			12		  	02          deleted
 * 		Ruby13		  	13	    		61			11		  	00          deleted
 ****************************************************************************/

#define AgentsRfidTag_h
class AgentsRfidTag
{
public:
    AgentsRfidTag();
    void Initialize(byte ruby);
    const char *badge;
    byte sector;                                    // 指定讀寫的「區段」，可能值:0~15，從區段15開始使用
    byte block;                                     // 指定讀寫的「區塊」，可能值:0~3，區塊3不使用
    byte blockData[16] = {'R', 'u', 'b', 'y', '0'}; // 最多可存入16個字元
                                                    // 若要清除區塊內容，請寫入16個 0
                                                    // byte blockData[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    byte buffer[18];                                // 暫存讀取區塊內容的陣列，MIFARE_Read()方法要求至少要18位元組空間，來存放16位元組。

    void ShowRubyName()
    {
        Serial.print(F("["));
        for (byte i = 0; i < 16; i++)
        {
            if (blockData[i] == 0)
                break;
            Serial.write(blockData[i]);
        }
        Serial.print(F("]"));
    }
    void ShowRubyBlock()
    {
        Serial.print(F("[Sector "));
        Serial.print(sector);
        Serial.print(F("] [Block "));
        Serial.print(block);
        Serial.print(F("]"));
    }
    void RecodeTime()
    {
    }

private:
};
AgentsRfidTag::AgentsRfidTag() {}
void AgentsRfidTag::Initialize(byte ruby)
{
    // Serial.print(F("Target is "));
    sector = 15 - (ruby - 1) / 3;
    block = (ruby - 1) % 3;
    if (ruby < 10)
        blockData[5] = ruby + 48; // convert to ASCII
    else
    {
        blockData[4] = 49;             // convert to ASCII
        blockData[5] = ruby % 10 + 48; // convert to ASCII
    }
    switch (ruby)
    {
    case 1:
        badge = "Arthur";
        break;
    case 2:
        badge = "Merlin";
        break;
    case 3:
        badge = "Lancelot";
        break;
    case 4:
        badge = "Galahad";
        break;
    case 5:
        badge = "Percival";
        break;
    case 6:
        badge = "Bors";
        break;
    case 7:
        badge = "Guinevere";
        break;
    case 8:
        badge = "Excalibur";
        break;
    case 9:
        badge = "SwordStone";
        break;
    case 10:
        badge = "Viviane";
        break;
    default:
        break;
    }
    // for (byte i = 0; i < 16; i++)
    // {
    //     if (blockData[i] == 0)
    //         break;
    //     Serial.write(blockData[i]);
    // }
    // Serial.print(F(" ["));
    // Serial.print(badge);
    // Serial.print(F("]"));
}