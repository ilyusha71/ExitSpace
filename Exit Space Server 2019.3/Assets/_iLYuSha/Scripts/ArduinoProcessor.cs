using System.Collections.Generic;
using ExitGames.Client.Photon;
using Photon.Pun;
using Photon.Realtime;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class ArduinoProcessor : MonoBehaviour
{
    private WakakaGalaxy galaxy;

    int countCommands;
    void Awake ()
    {
        // galaxy = FindObjectOfType<WakakaGalaxy> ();
    }

    void Update ()
    {
        if (Input.GetKeyDown (KeyCode.K))
            ArduinoController.ArduinoConnector.WriteLine ("R");

        countCommands = ArduinoController.queueCommand.Count;
        for (int i = 0; i < countCommands; i++)
        {
            Processing (ArduinoController.queueCommand.Dequeue ());
        }
    }

    string nowTime;
    public void Processing (string[] commands)
    {
        if (commands[0] == "Clock")
        {
            nowTime = commands[1];
        }
        else if (commands.Length > 1)
        {
            // 確認關卡
            // 確認時間

            Hashtable props = new Hashtable ();
            string pos = commands[0] + ":" + commands[1];
            if (commands[3] == "Unlock")
            {
                if (ExitSpaceData.IsStage1Entry (commands[0]))
                {
                props = new Hashtable
                { { PlayerCustomData.STAGE_1_TIME, nowTime }, { PlayerCustomData.MAZE_TIME, "Pause" }, { PlayerCustomData.TRAP_TIME, "Pause" }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                    };
                }
                else if (ExitSpaceData.IsDoor (commands[0]) || ExitSpaceData.IsChallengeBox (commands[0]))
                {
                    props = new Hashtable
                    { { PlayerCustomData.MAZE_TIME, "Pause" }, { PlayerCustomData.TRAP_TIME, "Pause" }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                    };
                }
                else if (ExitSpaceData.IsStage2Entry (commands[0]))
                {
                    props = new Hashtable
                    { { PlayerCustomData.STAGE_2_TIME, nowTime }, { PlayerCustomData.MAZE_TIME, "Pause" }, { PlayerCustomData.TRAP_TIME, "Pause" }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                    };
                }
                else if (ExitSpaceData.IsStage3Entry (commands[0]))
                {
                    props = new Hashtable
                    { { PlayerCustomData.STAGE_3_TIME, nowTime }, { PlayerCustomData.MAZE_TIME, "Pause" }, { PlayerCustomData.TRAP_TIME, "Pause" }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                    };
                }
                else if (ExitSpaceData.IsUnlockTrapDoor (commands[0]))
                {
                    props = new Hashtable
                    { { PlayerCustomData.MAZE_TIME, "Pause" }, { PlayerCustomData.TRAP_TIME, nowTime }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                    };
                }
                else if (ExitSpaceData.IsUnlockMazeDoor (commands[0]))
                {
                    props = new Hashtable
                    { { PlayerCustomData.MAZE_TIME, nowTime }, { PlayerCustomData.TRAP_TIME, "Pause" }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                    };
                }
                else if (ExitSpaceData.IsStage4Entry (commands[0]))
                {
                    props = new Hashtable
                    { { PlayerCustomData.STAGE_4_TIME, nowTime }, { PlayerCustomData.MAZE_TIME, "Pause" }, { PlayerCustomData.TRAP_TIME, "Pause" }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                    };
                }
                else
                {

                }
                ArduinoController.ArduinoConnector.WriteLine (commands[0] + "/Unlocked/");
            }
            else if (commands[3] == "Confer")
            {
                props = new Hashtable
                { { PlayerCustomData.FINISH_TIME, nowTime }, { PlayerCustomData.TITLE, commands[0] },
                };
                ArduinoController.ArduinoConnector.WriteLine (commands[0] + "/Conferred/");
            }
            else if (commands[3] == "Badget")
            {
                // ArduinoController.ArduinoConnector.WriteLine (commands[0] + "/Reset/");
            }
            else if (ExitSpaceData.IsChallengeBox (commands[0]))
            {
                props = new Hashtable
                { { PlayerCustomData.FINISH_TIME, nowTime }, { PlayerCustomData.MAZE_TIME, "Pause" }, { PlayerCustomData.TRAP_TIME, "Pause" }, { PlayerCustomData.LAST_TIME, nowTime }, { PlayerCustomData.LAST_POS, pos }
                };
            }
            // 地圖標記
            foreach (Player player in PhotonNetwork.PlayerList)
            {
                if (player.NickName == commands[2])
                {
                    object dddd;
                    if (props.TryGetValue (PlayerCustomData.LAST_TIME, out dddd))
                    {
                        Debug.Log (dddd);
                    }
                    player.SetCustomProperties (props);
                }
            }
        }

    }
}