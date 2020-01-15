/****************************************************************************
 * RFID Writer
 * 
 * v1.2.200113
 * 1. 寫入內容		    RUBY_NUMBER		ASCII		Sector		Block
* 		Wakaka Key		00		    	48			09~15		00~02
 * 		Ruby01			01	    		49			15		  	00
 * 		Ruby02		  	02	    		50			15		  	01
 * 		Ruby03	  		03	    		51			15	  		02
 * 		Ruby04	  		04	    		52			14		  	00
 * 		Ruby05	  		05	    		53			14	  		01
 * 		Ruby06	  		06	    		54			14		  	02
 * 		Ruby07		  	07	    		55			13		  	00
 * 		Ruby08		  	08	    		56			13		  	01
 * 		Ruby09		  	09	    		57			13		  	02
 * 		Ruby10		  	10	    		58			12		  	00
 * 		Ruby11		  	11	    		59			12		  	01
 * 		Ruby12		  	12	    		60			12		  	02
 * 		Ruby13		  	13	    		61			11		  	00
 ****************************************************************************/
#define RFID_h
class RFID
{
public:
    RFID();
    void Initialize(byte ruby);
    byte sector;                                    // 指定讀寫的「區段」，可能值:0~15，從區段15開始使用
    byte block;                                     // 指定讀寫的「區塊」，可能值:0~3，區塊3不使用
    byte blockData[16] = {'R', 'u', 'b', 'y', '0'}; // 最多可存入16個字元
                                                    // 若要清除區塊內容，請寫入16個 0
                                                    // byte blockData[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    byte buffer[18];                                // 暫存讀取區塊內容的陣列，MIFARE_Read()方法要求至少要18位元組空間，來存放16位元組。

private:
};
RFID::RFID() {}
void RFID::Initialize(byte ruby)
{
    Serial.print(F("Target is ["));
    sector = 15 - (ruby - 1) / 3;
    block = (ruby - 1) % 3;
    if (ruby < 10)
        blockData[5] = ruby + 48; // convert to ASCII
    else
    {
        blockData[4] = 49;             // convert to ASCII
        blockData[5] = ruby % 10 + 48; // convert to ASCII
    }
    for (byte i = 0; i < 16; i++)
    {
        if (blockData[i] == 0)
            break;
        Serial.write(blockData[i]);
    }
    Serial.print(F("] in Sector "));
    Serial.print(sector);
    Serial.print(F(" ,Block "));
    Serial.print(block);
    Serial.println();
}