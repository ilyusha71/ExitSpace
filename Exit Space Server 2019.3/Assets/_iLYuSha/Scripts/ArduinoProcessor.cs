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
    public static string nowChosenPlayer;

    public TextMeshProUGUI textNowTime, textPlayer, textStage1, textStage2, textStage3, textTrap, textMaze, textStage4,
    textFinish, textLastTime, textLastPos;
    public TextMeshProUGUI textStage1Countdown, textStage2Countdown, textStage3Countdown,
    textTrapCountdown, textMazeCountdown, textStage4Countdown;
    public Image[] checkBadge;
    public TextMeshProUGUI textChallenge, textTitle;

    private WakakaGalaxy galaxy;
    private string nowTime;
    private int countCommands;

    [Header ("Ground Server")]
    public bool timeoutSwitch;
    public GameObject groundPanel;
    public Transform groupAdnBtns;
    private Dictionary<string, Button> dicAdnBtns = new Dictionary<string, Button> ();
    public Transform groupXtdBtns;
    private Dictionary<string, Button> dicXtdBtns = new Dictionary<string, Button> ();
    private string ADN;
    public Toggle[] tglPresents;
    public TMP_InputField customCallbackTime, writeID;
    public Color32 clearColor, checkColor, readColor;

    void Awake ()
    {
        textNowTime.text = "";
        Clear ();

        Button[] btnADNs = groupAdnBtns.GetComponentsInChildren<Button> ();
        int countBtns = btnADNs.Length;
        for (int i = 0; i < countBtns; i++)
        {
            btnADNs[i].GetComponentsInChildren<Image> () [0].color = clearColor;
            string adn = btnADNs[i].name;
            if (!dicAdnBtns.ContainsKey (adn))
                dicAdnBtns.Add (adn, btnADNs[i]);
            btnADNs[i].onClick.AddListener (() =>
            {
                ADN = adn;
                Debug.Log (ADN);
            });
        }

        btnADNs = groupXtdBtns.GetComponentsInChildren<Button> ();
        countBtns = btnADNs.Length;
        for (int i = 0; i < countBtns; i++)
        {
            string adn = btnADNs[i].name;
            if (!dicXtdBtns.ContainsKey (adn))
                dicXtdBtns.Add (adn, btnADNs[i]);
        }
    }

    void Update ()
    {
        if (Input.GetKeyDown (KeyCode.F9))
            groundPanel.SetActive (!groundPanel.activeSelf);
        if (Input.GetKeyDown (KeyCode.LeftBracket))
            Check ();
        if (Input.GetKeyDown (KeyCode.RightBracket))
            Unlock ();
        countCommands = ArduinoController.queueCommand.Count;
        for (int i = 0; i < countCommands; i++)
        {
            Processing (ArduinoController.queueCommand.Dequeue ());
        }
    }

    public void Processing (string[] commands)
    {
        if (commands[0] == "Clock")
        {
            nowTime = commands[1];
            textNowTime.text = nowTime;
            ShowPlayerData (nowChosenPlayer);
            if (!PhotonNetwork.IsMasterClient)
                PhotonNetwork.SetMasterClient (PhotonNetwork.LocalPlayer);
            PhotonNetwork.LocalPlayer.SetCustomProperties (new Hashtable { { PlayerCustomData.LAST_TIME, nowTime } });
        }
        else if (commands.Length > 1)
        {
            if (commands[1] == "Callback")
            {
                if (dicAdnBtns.ContainsKey (commands[2]))
                    dicAdnBtns[commands[2]].GetComponentsInChildren<Image> () [0].color = checkColor;
                if (dicXtdBtns.ContainsKey (commands[2]))
                    dicXtdBtns[commands[2]].GetComponentsInChildren<Image> () [0].color = checkColor;
            }
            else if (commands.Length > 4)
            {
                if (dicAdnBtns.ContainsKey (commands[1]))
                    dicAdnBtns[commands[1]].GetComponentsInChildren<Image> () [0].color = readColor;
                if (dicXtdBtns.ContainsKey (commands[1]))
                    dicXtdBtns[commands[1]].GetComponentsInChildren<Image> () [0].color = readColor;
                // 確認關卡
                // 確認時間

                string nowSite = commands[1] + ":" + commands[2];
                Hashtable props = new Hashtable ();
                props.Add (PlayerCustomData.LAST_TIME, nowTime);
                props.Add (PlayerCustomData.LAST_SITE, nowSite);

                /**********************************************************************
                 * Z/2-U9-C7/0/KGB-952/Reset/1.2.3.4./
                 * commands[1] = 2-U9-C7
                 * commands[2] = 0
                 * commands[3] = KGB-952
                 * commands[4] = Reset
                 * commands[5] = Badge#
                 **********************************************************************/
                if (commands[4] == "Reset")
                {
                    props.Add (PlayerCustomData.STAGE_1_TIME, nowTime);
                    int countBadge = commands[5].Split ('.').Length;
                    for (int i = 0; i < countBadge - 1; i++)
                    {
                        props.Add (PlayerCustomData.BADGE[int.Parse (commands[5].Split ('.') [i]) - 1], true);
                    }
                }
                /**********************************************************************
                 * Z/2-U9-C7/0/KGB-952/Badge/1.2.3.4./
                 * commands[1] = 2-U9-C7
                 * commands[2] = 0
                 * commands[3] = KGB-952
                 * commands[4] = Badge
                 * commands[5] = Badge#
                 **********************************************************************/
                else if (commands[4] == "Badge")
                {
                    int countBadge = commands[5].Split ('.').Length;
                    for (int i = 0; i < countBadge - 1; i++)
                    {
                        props.Add (PlayerCustomData.BADGE[int.Parse (commands[5].Split ('.') [i]) - 1], true);
                    }
                }
                /**********************************************************************
                 * Z/2-U9-C7/0/KGB-952/Unlock/
                 * commands[1] = 2-U9-C7
                 * commands[2] = 0
                 * commands[3] = KGB-952
                 * commands[4] = Unlock
                 **********************************************************************/
                else if (commands[4] == "Unlock")
                {
                    if (ExitSpaceData.IsMerlinCabinet (commands[1]))
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked_1_U1_X/");
                    else if (ExitSpaceData.IsStage2Door (commands[1]))
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/IsStage2Door/");
                    else if (ExitSpaceData.IsStage2Entry (commands[1]))
                    {
                        props.Add (PlayerCustomData.STAGE_2_TIME, nowTime);
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/IsStage2Entry/");
                    }
                    else if (ExitSpaceData.IsStage3Entry (commands[1]))
                    {
                        props.Add (PlayerCustomData.STAGE_3_TIME, nowTime);
                        if (commands[1] == "3-E6-E4")
                            ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked_3_E6_E4/IsStage3Entry/" + commands[2] + "/");
                        else if (commands[1] == "3-U1-X")
                            ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/IsStage3Entry/");
                    }
                    else if (ExitSpaceData.IsChanceDoor (commands[1]))
                    {
                        props.Add (PlayerCustomData.MAZE_TIME, "Pause");
                        props.Add (PlayerCustomData.TRAP_TIME, nowTime);
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/IsChanceDoor/");
                    }
                    else if (ExitSpaceData.IsFateDoor (commands[1]))
                    {
                        props.Add (PlayerCustomData.MAZE_TIME, nowTime);
                        props.Add (PlayerCustomData.TRAP_TIME, "Pause");
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/IsFateDoor/");
                    }
                    else if (ExitSpaceData.IsStage4Entry (commands[1]))
                    {
                        props.Add (PlayerCustomData.STAGE_4_TIME, nowTime);
                        props.Add (PlayerCustomData.MAZE_TIME, "Pause");
                        props.Add (PlayerCustomData.TRAP_TIME, "Pause");
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/IsStage4Entry/");
                    }
                    else if (ExitSpaceData.IsChallengeBox (commands[1]))
                    {
                        string title = ExitSpaceData.GetTitle (commands[1].Split ('-') [2]);
                        props.Add (PlayerCustomData.CHALLENGE, title);
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/IsBox/");
                    }
                    else
                    {
                        Debug.LogWarning ("Site: " + commands[1]);
                        ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Unlocked/Unknown");
                    }
                }
                /**********************************************************************
                 * Z/4B1-A345-T1/0/KGB-952/Confer/
                 * commands[1] = 4B1-A345-T1
                 * commands[2] = 0
                 * commands[3] = KGB-952
                 * commands[4] = Confer
                 **********************************************************************/
                else if (commands[4] == "Confer")
                {
                    string title = ExitSpaceData.GetTitle (commands[1].Split ('-') [2]);
                    props.Add (PlayerCustomData.TITLE, title);
                    props.Add (PlayerCustomData.FINISH_TIME, nowTime);
                    ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Conferred/");
                }
                // 地圖標記
                foreach (Player player in PhotonNetwork.PlayerList)
                {
                    if (player.NickName == commands[3])
                    {
                        // object dddd;
                        // if (props.TryGetValue (PlayerCustomData.LAST_TIME, out dddd))
                        // {
                        //     Debug.Log (dddd);
                        // }
                        player.SetCustomProperties (props);
                    }
                }
            }
        }
    }

    public void ShowPlayerData (string id)
    {
        Clear ();
        nowChosenPlayer = id;
        foreach (Player player in PhotonNetwork.PlayerList)
        {
            if (player.NickName == id)
            {
                textPlayer.text = player.NickName;
                object customData;
                if (player.CustomProperties.TryGetValue (PlayerCustomData.LAST_TIME, out customData))
                    textLastTime.text = (string) customData;
                if (player.CustomProperties.TryGetValue (PlayerCustomData.LAST_SITE, out customData))
                    textLastPos.text = (string) customData;
                string lastPos = (string) customData;
                if (player.CustomProperties.TryGetValue (PlayerCustomData.STAGE_1_TIME, out customData))
                {
                    string startTime = (string) customData;
                    textStage1.text = (string) customData;
                    if (player.CustomProperties.TryGetValue (PlayerCustomData.STAGE_2_TIME, out customData))
                    {
                        textStage2.text = (string) customData;
                        if (player.CustomProperties.TryGetValue (PlayerCustomData.STAGE_3_TIME, out customData))
                        {
                            textStage3.text = (string) customData;
                            if (player.CustomProperties.TryGetValue (PlayerCustomData.STAGE_4_TIME, out customData))
                            {
                                textStage4.text = (string) customData;
                                if (player.CustomProperties.TryGetValue (PlayerCustomData.CHALLENGE, out customData))
                                    textChallenge.text = (string) customData;
                                if (player.CustomProperties.TryGetValue (PlayerCustomData.FINISH_TIME, out customData))
                                {
                                    textFinish.text = (string) customData;
                                    if (player.CustomProperties.TryGetValue (PlayerCustomData.TITLE, out customData))
                                        textTitle.text = (string) customData;
                                }
                                else
                                    textStage4Countdown.text = GetCountdown (startTime, PlayerCustomData.STAGE_4_LIMIT).ToString ();
                            }
                            else
                            {
                                textStage3Countdown.text = GetCountdown (startTime, PlayerCustomData.STAGE_3_LIMIT).ToString ();
                                if (player.CustomProperties.TryGetValue (PlayerCustomData.TRAP_TIME, out customData))
                                {
                                    string trapTime = (string) customData;
                                    textTrap.text = trapTime;
                                    if (trapTime != "Pause")
                                    {
                                        if (ExitSpaceData.IsChance18Door (lastPos))
                                            textTrapCountdown.text = GetCountdown (trapTime, PlayerCustomData.TRAP_18_LIMIT).ToString ();
                                        else
                                            textTrapCountdown.text = GetCountdown (trapTime, PlayerCustomData.TRAP_LIMIT).ToString ();
                                    }
                                }
                                if (player.CustomProperties.TryGetValue (PlayerCustomData.MAZE_TIME, out customData))
                                {
                                    string mazeTime = (string) customData;
                                    textMaze.text = mazeTime;
                                    if (mazeTime != "Pause")
                                        textMazeCountdown.text = GetCountdown (mazeTime, PlayerCustomData.MAZE_LIMIT).ToString ();
                                }
                            }
                        }
                        else
                            textStage2Countdown.text = GetCountdown (startTime, PlayerCustomData.STAGE_2_LIMIT).ToString ();
                    }
                    else
                        textStage1Countdown.text = GetCountdown (startTime, PlayerCustomData.STAGE_1_LIMIT).ToString ();
                }

                for (int i = 0; i < checkBadge.Length; i++)
                {
                    if (player.CustomProperties.TryGetValue (PlayerCustomData.BADGE[i], out customData))
                        checkBadge[i].enabled = (bool) customData;
                }

                if (player.CustomProperties.TryGetValue (PlayerCustomData.CHALLENGE, out customData))
                    textChallenge.text = (string) customData;
                if (player.CustomProperties.TryGetValue (PlayerCustomData.TITLE, out customData))
                    textTitle.text = (string) customData;
            }
        }
    }
    private void Clear ()
    {
        textPlayer.text = "";
        textStage1.text = "";
        textStage1Countdown.text = "";
        textStage2.text = "";
        textStage2Countdown.text = "";
        textStage3.text = "";
        textStage3Countdown.text = "";
        textTrap.text = "";
        textTrapCountdown.text = "";
        textMaze.text = "";
        textMazeCountdown.text = "";
        textStage4.text = "";
        textStage4Countdown.text = "";
        textFinish.text = "";
        textLastTime.text = "";
        textLastPos.text = "";
        for (int i = 0; i < checkBadge.Length; i++)
        {
            checkBadge[i].enabled = false;
        }
        textChallenge.text = "";
        textTitle.text = "";
    }

    private int GetCountdown (string time, int limit)
    {
        return limit - (3600 * (int.Parse (nowTime.Split (':') [0]) - int.Parse (time.Split (':') [0])) +
            60 * (int.Parse (nowTime.Split (':') [1]) - int.Parse (time.Split (':') [1])) +
            (int.Parse (nowTime.Split (':') [2]) - int.Parse (time.Split (':') [2])));
    }

    public void Unlock ()
    {
        ArduinoController.ArduinoConnector.WriteLine ("Z/" + ADN + "/UnlockForce/");
    }
    public void Check ()
    {
        string callback = "2000";
        if (ExitSpaceData.IsWriter (ADN))
            callback = ExitSpaceData.GetWriterCallback (ADN);
        else if (ExitSpaceData.IsChallengeBox (ADN))
            callback = ExitSpaceData.GetBoxCallback (ADN);
        if (callback == "2000")
            callback = customCallbackTime.text;
        ArduinoController.ArduinoConnector.WriteLine ("Z/" + ADN + "/Checking/" + callback + "/");
    }
    public void CheckRestart ()
    {
        foreach (Button btn in dicAdnBtns.Values)
        {
            btn.GetComponentsInChildren<Image> () [0].color = clearColor;
        }
    }
    public void SendBadgePresents ()
    {
        if (!ExitSpaceData.IsStage1Entry (ADN)) return;
        string presents = "";
        for (int i = 0; i < tglPresents.Length; i++)
        {
            presents += tglPresents[i].isOn ? ((i + 1) + ".") : "";
        }
        Debug.Log (presents);
        ArduinoController.ArduinoConnector.WriteLine ("Z/" + ADN + "/Present/" + presents + "/");
    }
    public void WriteNewAgentID ()
    {
        ArduinoController.ArduinoConnector.WriteLine ("Z/" + ADN + "/ID/" + writeID.text + "/");
    }

    /**********************************************************************
     * Dashboard Command
     **********************************************************************/

}