using System.Collections.Generic;
using DG.Tweening;
using ExitGames.Client.Photon;
using Photon.Pun;
using Photon.Realtime;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class AgentCampManager : MonoBehaviour
{
    [Header ("SFX")]
    public AudioClip clipCallbackBell;
    private AudioSource audioSource;

    public Player nowChosenPlayer;

    public TextMeshProUGUI textNowTime, textPlayer, textStage1, textStage2, textStage3, textTrap, textMaze, textStage4,
    textFinish, textLastTime, textLastPos;
    public TextMeshProUGUI textStage1Countdown, textStage2Countdown, textStage3Countdown,
    textTrapCountdown, textMazeCountdown, textStage4Countdown;
    public Image[] checkBadge;
    public TextMeshProUGUI textChallenge, textTitle;

    private WakakaGalaxy galaxy;
    private int countCommands;

    [Header ("Ground Server")]
    public TextMeshProUGUI textVersion;
    public Transform groupAdnBtns;
    private Dictionary<string, Button> dicAdnBtns = new Dictionary<string, Button> ();
    public Transform groupXtdBtns;
    private Dictionary<string, Button> dicXtdBtns = new Dictionary<string, Button> ();
    private string ADN;
    public Toggle[] tglPresents;
    public TMP_InputField customCallbackTime, writeID;
    public Color32 clearColor, checkColor, readColor;

    [Header ("Auto Checking")]
    public Toggle isChecking;
    public Button[] btnStages;
    public string[] checkList = new string[0];
    public Animator TimedStupidityBomb, Timespace;
    public TextMeshProUGUI textDevice, textCountdown;
    public string autoCheckingTarget;
    public float nextCheckingTime;
    public int indexChecking;
    [Header ("Warning")]
    public GameObject warning;
    public Button forceQuit;

    [Header ("Black Door")]
    public List<string> blackDoor = new List<string> ();

    public GameObject gg;

    [Header ("Canvas")]
    public Transform content, ground;
    private float offset;

    [Header ("Pinball")]
    public Button[] pinball;
    public TextMeshProUGUI[] avgVolt;
    public TMP_InputField inputDec, inputInter;

    void Awake ()
    {
        audioSource = GetComponent<AudioSource> ();
        textVersion.text = Application.version;
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

        // Auto Checking
        isChecking.onValueChanged.AddListener ((isOn) =>
        {
            if (isOn)
            {
                // textDevice.text = checkList[indexChecking];
                TimedStupidityBomb.speed = 0;
                Timespace.speed = 1;
            }
            else
            {
                textDevice.text = "Pause";
                TimedStupidityBomb.speed = 1;
                Timespace.speed = 0;
            }
        });
        btnStages[0].onClick.AddListener (() =>
        {
            checkList = ExitSpaceData.STAGE_1_DEVICES;
        });
        btnStages[1].onClick.AddListener (() =>
        {
            checkList = ExitSpaceData.STAGE_2_DEVICES;
        });
        btnStages[2].onClick.AddListener (() =>
        {
            checkList = ExitSpaceData.STAGE_3_DEVICES;
        });
        btnStages[3].onClick.AddListener (() =>
        {
            checkList = ExitSpaceData.STAGE_4_DEVICES;
        });

        DisableAutoChecking ();

        if (!isChecking.enabled)
        {
            nextCheckingTime = Time.time + 4.0f;
            indexChecking = checkList.Length - 1;
            isChecking.enabled = true;
            textDevice.text = "Are U Ready?";
        }
        // offset = 540 + 0.5f * (1920 - Screen.width * 1080 / Screen.height);
        offset = 1080 + (1920 - Screen.width * 1080 / Screen.height);

        for (int i = 0; i < 3; i++)
        {
            int index = i;
            pinball[i].onClick.AddListener (() =>
            {
                ADN = "Pinball/" + index.ToString ();
                Debug.Log (ADN);
            });
        }
    }

    void DisableAutoChecking ()
    {
        isChecking.enabled = false;
        isChecking.isOn = false;
        textDevice.text = "Disconnected";
        TimedStupidityBomb.speed = 0;
        Timespace.speed = 0;
    }

    void Update ()
    {
        if (Input.GetKeyDown (KeyCode.Alpha1))
        {
            content.DOKill ();
            content.DOLocalMoveY (-3830, 1.37f);
        }
        else if (Input.GetKeyDown (KeyCode.Alpha2))
        {
            content.DOKill ();
            content.DOLocalMoveY (0, 1.37f);
        }
        else if (Input.GetKeyDown (KeyCode.Alpha3))
        {
            content.DOKill ();
            content.DOLocalMoveY (1560, 1.37f);
        }
        content.Translate (new Vector3 (Input.GetAxis ("Horizontal") * -5.0f, 0, 0));
        if (content.localPosition.x > 0)
            content.localPosition = new Vector3 (0, content.localPosition.y, content.localPosition.z);
        else if (content.localPosition.x < -offset)
            content.localPosition = new Vector3 (-offset, content.localPosition.y, content.localPosition.z);

        if (Input.GetKeyDown (KeyCode.LeftBracket))
            Check ();
        if (Input.GetKeyDown (KeyCode.RightBracket))
            Unlock ();
        countCommands = ArduinoController.queueCommand.Count;
        for (int i = 0; i < countCommands; i++)
        {
            Processing (ArduinoController.queueCommand.Dequeue ());
        }

        if (isChecking.isOn && checkList.Length > 0)
        {
            if (Time.time > nextCheckingTime)
            {
                nextCheckingTime = Time.time + 5.0f;
                indexChecking = (int) Mathf.Repeat (++indexChecking, checkList.Length);
                textDevice.text = checkList[indexChecking];

                PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
                { { PlayerCustomData.COMMAND, "Device/" + checkList[indexChecking] + "/Checking/100/" + PhotonNetwork.LocalPlayer.NickName + "/" }
                });
            }
            textCountdown.text = ((int) (nextCheckingTime - Time.time)).ToString ();
        }
        else
            textCountdown.text = "---";
    }

    public void Processing (string[] commands)
    {
        if (commands[0] == "Clock" || commands[0] == "DS3231")
        {
            textNowTime.text = ArduinoDashboard.localTime;;
            ShowPlayerData ();
            // PhotonNetwork.LocalPlayer.SetCustomProperties (new Hashtable { { PlayerCustomData.LAST_TIME, ArduinoDashboard.nowTime } });

            //temp
            Player[] players = PhotonNetwork.PlayerList;
            int participants = 0, badges = 0;
            bool checkArthur = false;
            for (int i = 0; i < players.Length; i++)
            {
                object customData;
                if (players[i].NickName.Contains ("CIA") || players[i].NickName.Contains ("KGB"))
                {
                    participants++;
                    for (int j = 0; j < 10; j++)
                    {
                        if (players[i].CustomProperties.TryGetValue (PlayerCustomData.BADGE[j], out customData))
                            if ((bool) customData)
                            {
                                badges++;
                                if (j == 0)
                                    checkArthur = true;
                            }

                    }
                }
            }
            // Debug.Log ("participants: " + participants + "   badges: " + badges);
            if (badges > participants * 5.6f && checkArthur)
                ArduinoController.ArduinoConnector.WriteLine ("Z/BadgeGate/Pass/");
        }
        else if (commands.Length > 1)
        {
            if (commands[1] == "Callback")
            {
                if (dicAdnBtns.ContainsKey (commands[2]))
                {
                    dicAdnBtns[commands[2]].GetComponentsInChildren<Image> () [0].color = checkColor;
                    dicAdnBtns[commands[2]].GetComponentsInChildren<Image> () [1].enabled = true;
                    audioSource.PlayOneShot (clipCallbackBell);
                    nextCheckingTime = Time.time;
                }
                if (dicXtdBtns.ContainsKey (commands[2]))
                {
                    dicXtdBtns[commands[2]].GetComponentsInChildren<Image> () [0].color = checkColor;
                    dicXtdBtns[commands[2]].GetComponentsInChildren<Image> () [1].enabled = true;
                    audioSource.PlayOneShot (clipCallbackBell);
                    nextCheckingTime = Time.time;
                }
            }
            else if (commands.Length > 4)
            {
                if (PhotonNetwork.LocalPlayer.NickName == "MASTER")
                {
                    if (isChecking.isOn)
                    {
                        warning.SetActive (true);
                        forceQuit.onClick.AddListener (() =>
                        {
                            Application.Quit ();
                        });
                    }
                }
                if (commands[2] != "Checking")
                {
                    if (dicAdnBtns.ContainsKey (commands[1]))
                        dicAdnBtns[commands[1]].GetComponentsInChildren<Image> () [0].color = readColor;
                    if (dicXtdBtns.ContainsKey (commands[1]))
                        dicXtdBtns[commands[1]].GetComponentsInChildren<Image> () [0].color = readColor;
                }
                // 確認關卡
                // 確認時間

                string nowSite = commands[1] + ":" + commands[2];
                Hashtable props = new Hashtable ();
                props.Add (PlayerCustomData.LAST_TIME, ArduinoDashboard.localTime);
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
                    props.Add (PlayerCustomData.STAGE_1_TIME, ArduinoDashboard.localTime);
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
                    string unlockCommand = "";
                    if (blackDoor.Contains (commands[1]))
                        unlockCommand = ("Z/BLACK/Unlocked");
                    else if (ExitSpaceData.IsMerlinCabinet (commands[1]))
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked_1_U1_X/");
                    else if (ExitSpaceData.IsStage2Door (commands[1]))
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked/IsStage2Door/");
                    else if (ExitSpaceData.IsStage2Entry (commands[1]))
                    {
                        props.Add (PlayerCustomData.STAGE_2_TIME, ArduinoDashboard.localTime);
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked/IsStage2Entry/");
                    }
                    else if (ExitSpaceData.IsStage3Entry (commands[1]))
                    {
                        props.Add (PlayerCustomData.STAGE_3_TIME, ArduinoDashboard.localTime);
                        if (commands[1] == "3-E6-E4")
                            unlockCommand = ("Z/" + commands[1] + "/Unlocked_3_E6_E4/IsStage3Entry/" + commands[2] + "/");
                        else if (commands[1] == "3-U1-X")
                            unlockCommand = ("Z/" + commands[1] + "/Unlocked/IsStage3Entry/");
                    }
                    else if (ExitSpaceData.IsChanceDoor (commands[1]))
                    {
                        props.Add (PlayerCustomData.MAZE_TIME, "Pause");
                        props.Add (PlayerCustomData.TRAP_TIME, ArduinoDashboard.localTime);
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked/IsChanceDoor/");
                    }
                    else if (ExitSpaceData.IsFateDoor (commands[1]))
                    {
                        props.Add (PlayerCustomData.MAZE_TIME, ArduinoDashboard.localTime);
                        props.Add (PlayerCustomData.TRAP_TIME, "Pause");
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked/IsFateDoor/");
                    }
                    else if (ExitSpaceData.IsStage4Entry (commands[1]))
                    {
                        props.Add (PlayerCustomData.STAGE_4_TIME, ArduinoDashboard.localTime);
                        props.Add (PlayerCustomData.MAZE_TIME, "Pause");
                        props.Add (PlayerCustomData.TRAP_TIME, "Pause");
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked/IsStage4Entry/");
                    }
                    else if (ExitSpaceData.IsChallengeBox (commands[1]))
                    {
                        string title = ExitSpaceData.GetTitle (commands[1].Split ('-') [2]);
                        props.Add (PlayerCustomData.CHALLENGE, title);
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked/IsBox/");
                    }
                    else
                    {
                        Debug.LogWarning ("Site: " + commands[1]);
                        unlockCommand = ("Z/" + commands[1] + "/Unlocked/Unknown");
                    }
                    ArduinoController.ArduinoConnector.WriteLine (unlockCommand);
                    ArduinoDashboard.Instance.ArduinoTransmittedMessage.AddMessage (unlockCommand);
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
                    props.Add (PlayerCustomData.FINISH_TIME, ArduinoDashboard.localTime);
                    ArduinoController.ArduinoConnector.WriteLine ("Z/" + commands[1] + "/Conferred/");
                    ArduinoDashboard.Instance.ArduinoTransmittedMessage.AddMessage (commands[1] + "/Conferred/");
                }
                /**********************************************************************
                 * Z/Pinball/0/KGB-952/Reset/1.2.3.4./
                 * commands[1] = 2-U9-C7
                 * commands[2] = 0
                 * commands[3] = KGB-952
                 * commands[4] = Reset
                 * commands[5] = Badge#
                 **********************************************************************/
                else if (commands[1] == "Pinball")
                {
                    switch (commands[2])
                    {
                        case "0":
                            break;
                        case "1":
                            break;
                        case "2":
                            break;
                        case "avg":
                            avgVolt[0].text = commands[3];
                            avgVolt[1].text = commands[4];
                            avgVolt[2].text = commands[5];
                            break;
                    }

                    // props.Add (PlayerCustomData.STAGE_1_TIME, ArduinoDashboard.localTime);
                    // int countBadge = commands[5].Split ('.').Length;
                    // for (int i = 0; i < countBadge - 1; i++)
                    // {
                    //     props.Add (PlayerCustomData.BADGE[int.Parse (commands[5].Split ('.') [i]) - 1], true);
                    // }
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
                PhotonNetwork.MasterClient.SetCustomProperties (
                    new Hashtable
                    {
                        {
                            PlayerCustomData.MASTER_CB, commands
                        }
                    });
            }
        }
    }

    /**********************************************************************
     * Player Manage
     **********************************************************************/
    public void ChoosePlayer (string nickName)
    {
        foreach (Player player in PhotonNetwork.PlayerList)
        {
            if (player.NickName == nickName)
            {
                nowChosenPlayer = player;
            }
        }
    }

    public void ShowPlayerData ()
    {
        if (nowChosenPlayer == null) return;
        Clear ();
        textPlayer.text = nowChosenPlayer.NickName;
        object customData;
        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.LAST_TIME, out customData))
            textLastTime.text = (string) customData;
        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.LAST_SITE, out customData))
            textLastPos.text = (string) customData;
        string lastPos = (string) customData;
        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.STAGE_1_TIME, out customData))
        {
            string startTime = (string) customData;
            textStage1.text = (string) customData;
            if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.STAGE_2_TIME, out customData))
            {
                textStage2.text = (string) customData;
                if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.STAGE_3_TIME, out customData))
                {
                    textStage3.text = (string) customData;
                    if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.STAGE_4_TIME, out customData))
                    {
                        textStage4.text = (string) customData;
                        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.CHALLENGE, out customData))
                            textChallenge.text = (string) customData;
                        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.FINISH_TIME, out customData))
                        {
                            textFinish.text = (string) customData;
                            if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.TITLE, out customData))
                                textTitle.text = (string) customData;
                        }
                        else
                            textStage4Countdown.text = GetCountdown (startTime, PlayerCustomData.STAGE_4_LIMIT).ToString ();
                    }
                    else
                    {
                        textStage3Countdown.text = GetCountdown (startTime, PlayerCustomData.STAGE_3_LIMIT).ToString ();
                        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.TRAP_TIME, out customData))
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
                        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.MAZE_TIME, out customData))
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
            if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.BADGE[i], out customData))
                checkBadge[i].enabled = (bool) customData;
        }

        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.CHALLENGE, out customData))
            textChallenge.text = (string) customData;
        if (nowChosenPlayer.CustomProperties.TryGetValue (PlayerCustomData.TITLE, out customData))
            textTitle.text = (string) customData;
    }

    public void ToBeTheMasterClient ()
    {
        PhotonNetwork.SetMasterClient (PhotonNetwork.LocalPlayer);
    }

    public void TakeOverAsNewMasterClient ()
    {
        PhotonNetwork.SetMasterClient (nowChosenPlayer);
    }

    public void KickPlayer ()
    {
        if (PhotonNetwork.MasterClient != PhotonNetwork.LocalPlayer || nowChosenPlayer == null)
            gg.SetActive (true);
        else
            PhotonNetwork.CloseConnection (nowChosenPlayer);
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
        return limit - (3600 * (int.Parse (ArduinoDashboard.localTime.Split (':') [0]) - int.Parse (time.Split (':') [0])) +
            60 * (int.Parse (ArduinoDashboard.localTime.Split (':') [1]) - int.Parse (time.Split (':') [1])) +
            (int.Parse (ArduinoDashboard.localTime.Split (':') [2]) - int.Parse (time.Split (':') [2])));
    }

    /**********************************************************************
     * Dashboard Command
     **********************************************************************/
    /// <summary>
    /// 寫入卡片識別ID
    /// </summary>
    public void WriteNewAgentID ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        { { PlayerCustomData.COMMAND, "Device/" + ADN + "/ID/" + writeID.text + "/" + PhotonNetwork.LocalPlayer.NickName + "/" }
        });
    }
    /// <summary>
    /// 送徽章
    /// </summary>
    public void SendBadgePresents ()
    {
        if (!ExitSpaceData.IsStage1Entry (ADN)) return;
        string presents = "";
        for (int i = 0; i < tglPresents.Length; i++)
        {
            presents += tglPresents[i].isOn ? ((i + 1) + ".") : "";
        }

        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        { { PlayerCustomData.COMMAND, "Device/" + ADN + "/Present/" + presents + "/" + PhotonNetwork.LocalPlayer.NickName + "/" }
        });
    }
    /// <summary>
    /// 強制解鎖
    /// </summary>
    public void Unlock ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        { { PlayerCustomData.COMMAND, "Device/" + ADN + "/UnlockForce/" + PhotonNetwork.LocalPlayer.NickName + "/" }
        });
    }
    /// <summary>
    /// HC12雙向通訊檢測
    /// </summary>
    public void Check ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        { { PlayerCustomData.COMMAND, "Device/" + ADN + "/Checking/" + customCallbackTime.text + "/" + PhotonNetwork.LocalPlayer.NickName + "/" }
        });
    }
    /// <summary>
    /// 設定光敏電阻觸發臨界值
    /// 預設為50
    /// </summary>
    public void SetDeclineFactor ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        { { PlayerCustomData.COMMAND, "Device/" + ADN + "/SetDecline/" + inputDec.text + "/" + PhotonNetwork.LocalPlayer.NickName + "/" }
        });
    }
    /// <summary>
    /// 設定光敏電阻觸發兩次之間隔時間
    /// 預設為500ms
    /// </summary>
    public void SetInterval ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        { { PlayerCustomData.COMMAND, "Device/" + ADN + "/SetInterval/" + inputInter.text + "/" + PhotonNetwork.LocalPlayer.NickName + "/" }
        });
    }

    public void AddBlackDoor ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        { { PlayerCustomData.COMMAND, "Master/" + ADN + "/BlockDoor/" + PhotonNetwork.LocalPlayer.NickName + "/" }
        });
    }
    public void CheckRestart ()
    {
        foreach (Button btn in dicAdnBtns.Values)
        {
            btn.GetComponentsInChildren<Image> () [0].color = clearColor;
            btn.GetComponentsInChildren<Image> () [1].enabled = false;
        }
        foreach (Button btn in dicXtdBtns.Values)
        {
            btn.GetComponentsInChildren<Image> () [0].color = clearColor;
            btn.GetComponentsInChildren<Image> () [1].enabled = false;
        }
    }
    public void Plus (int num)
    {
        indexChecking += num;
        indexChecking = (int) Mathf.Repeat (indexChecking, checkList.Length);
        nextCheckingTime = Time.time;
    }
    /**********************************************************************
     * Developer & Master
     **********************************************************************/
    public void DeveloperCommand ()
    {
        object customData;
        if (PhotonNetwork.MasterClient.CustomProperties.TryGetValue (PlayerCustomData.COMMAND, out customData))
        {
            if (((string) customData).Contains ("Device")) // 傳送給其他Arduino的指令
            {
                ArduinoController.ArduinoConnector.WriteLine ((string) customData);
                ArduinoDashboard.Instance.ArduinoTransmittedMessage.AddMessage ((string) customData);
                PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable { { PlayerCustomData.COMMAND, "" } });
            }
            else if (((string) customData).Contains ("Master")) // 要求MasterClient端執行的指令
            {
                ArduinoDashboard.Instance.ArduinoReceivedlMessage.AddMessage ((string) customData);
                if (((string) customData).Contains ("BlackDoor"))
                    blackDoor.Add (((string) customData).Split ('/') [1]);
                PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable { { PlayerCustomData.COMMAND, "" } });

            }
        }
    }
    public void MasterCallback ()
    {
        object customData;

        if (PhotonNetwork.MasterClient.CustomProperties.TryGetValue (PlayerCustomData.MASTER_CB, out customData))
        {

        }
    }
}