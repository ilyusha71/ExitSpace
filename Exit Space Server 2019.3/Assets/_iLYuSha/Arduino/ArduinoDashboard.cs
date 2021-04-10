using System;
using System.Collections.Generic;
using ExitGames.Client.Photon;
using Photon.Pun;
using TMPro;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class ArduinoDashboard : MonoBehaviour
{
    public static ArduinoDashboard Instance { get; private set; }

    [Header ("Dashboard")]
    public bool hasLanding;
    public GameObject mainPanel;
    public PlayableDirector Top2Down, Down2Top;

    [Header ("SFX")]
    public AudioClip clipToggleClick;
    private AudioSource audioSource;

    [Header ("UI - Arduino Messages")]
    public MessageManager ArduinoReceivedlMessage;
    public MessageManager ArduinoTransmittedMessage;

    [Header ("UI - Arduino Serial Port Setting")]
    public bool isDontDestroy;
    public GameObject listPort;
    public GameObject listBaud;
    private Toggle[] portOption;
    private Toggle[] baudOption;
    private float timesPressDelete { get; set; }
    private float timesPressESC { get; set; }
    private int countBuffer; // 緩存計數器

    [Header ("UI - Event")]
    public Button btnConnect;
    public Button btnDisconnect;
    private TextMeshProUGUI textQuit;
    public TextMeshProUGUI textTime;

    [Header ("UI - Master/Server Setting")]
    public static string localTime;
    public static int lag = -999;
    public TextMeshProUGUI localClock;
    public TextMeshProUGUI masterClock;
    public TextMeshProUGUI ds3231Clock;
    public TMP_InputField inputSynchronization, inputTransmission;

    /**********************************************************************
     * 2020/07/26 更新 Arduino Dashboard
     **********************************************************************/

    [Header ("UI - Message")]
    public int countCommand;
    private int countMsg;

    void Awake ()
    {
        Instance = this;
        if (isDontDestroy)
            DontDestroyOnLoad (this);
        audioSource = GetComponent<AudioSource> ();
        ArduinoReceivedlMessage.Initialize ();
        ArduinoTransmittedMessage.Initialize ();

        portOption = listPort.GetComponentsInChildren<Toggle> ();
        int countPort = portOption.Length;
        for (int i = 0; i < countPort; i++)
        {
            int index = i;
            portOption[index].onValueChanged.AddListener (isOn =>
            {
                if (isOn)
                {
                    audioSource.PlayOneShot (clipToggleClick);
                    portOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 1f;
                    ArduinoController.Port = index;
                    ArduinoReceivedlMessage.AddMessage ("[Control]Serial port has been reset to " + ArduinoController.SerialPort);
                }
                else
                    portOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 0.27f;
            });
        }
        baudOption = listBaud.GetComponentsInChildren<Toggle> ();
        int countRate = baudOption.Length;
        for (int i = 0; i < countRate; i++)
        {
            int index = i;
            baudOption[index].onValueChanged.AddListener (isOn =>
            {
                if (isOn)
                {
                    audioSource.PlayOneShot (clipToggleClick);
                    baudOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 1f;
                    ArduinoController.Baud = index;
                    ArduinoReceivedlMessage.AddMessage ("[Control]Serial baud has been reset to " + ArduinoController.SerialBaud);
                }
                else
                    baudOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 0.36f;
            });
        }
        portOption[ArduinoController.Port].isOn = true;
        baudOption[ArduinoController.Baud].isOn = true;

        btnConnect.onClick.AddListener (ArduinoController.ConnectArduino);
        btnDisconnect.onClick.AddListener (() =>
        {
            if (ArduinoController.Status == ArduinoStatus.Unconnected)
                Application.Quit ();
            else if (ArduinoController.Status == ArduinoStatus.Connected)
                ArduinoController.DisconnectArduino ();
        });
        textQuit = btnDisconnect.GetComponentInChildren<TextMeshProUGUI> ();
        ArduinoController.countCommand = countCommand;
    }

    void Update ()
    {
        localTime =
            String.Format ("{0:00}", DateTime.Now.Hour) + ":" +
            String.Format ("{0:00}", DateTime.Now.Minute) + ":" +
            String.Format ("{0:00}", DateTime.Now.Second);
        localClock.text = localTime;
        if (lag != -999)
        {
            int second = DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second + lag;
            masterClock.text =
                String.Format ("{0:00}", second / 3600) + ":" +
                String.Format ("{0:00}", (second % 3600) / 60) + ":" +
                String.Format ("{0:00}", second % 60);
        }

        textTime.text = ArduinoController.timeBoot + "\n" + ArduinoController.timeLastReceived + "\n" + localTime;

        if (Input.GetKey (KeyCode.LeftControl) && Input.GetKey (KeyCode.LeftAlt))
        {
            if (Input.GetKeyDown (KeyCode.Slash))
                ArduinoController.ConnectArduino ();
        }

        /*************************************************
         *   Main Thread Messages
         *************************************************/
        countMsg = ArduinoController.queueMsg.Count;
        for (int i = 0; i < countMsg; i++)
        {
            countBuffer++;
            string msgRx = ArduinoController.queueMsg.Dequeue ();
            if (msgRx.Contains ("DS3231") || msgRx.Contains ("Clock"))
                ds3231Clock.text = msgRx.Split ('/') [1];
            else if (!msgRx.Contains ("Device"))
                ArduinoReceivedlMessage.AddMessage (msgRx);
        }
        if (countBuffer % 30000 == 0)
        {
            countBuffer++;
            GC.Collect ();
            ArduinoReceivedlMessage.AddMessage ("[Control]Memory has been deallocated.");
        }
        ArduinoController.CheckStatus ();
        if (ArduinoController.Status == ArduinoStatus.Connected)
            textQuit.text = "Disconnect";
        else if (ArduinoController.Status == ArduinoStatus.Unconnected)
            textQuit.text = "Quit";

        // 刪除設定
        if (Input.GetKey (KeyCode.Delete))
        {
            timesPressDelete++;
            if (timesPressDelete > 150)
            {
                ArduinoController.DeletePrefs ();
                portOption[ArduinoController.Port].isOn = true;
                baudOption[ArduinoController.Baud].isOn = true;
                ArduinoReceivedlMessage.AddMessage ("[Control]Your settings have been deleted.");
            }
        }
        else if (Input.GetKeyUp (KeyCode.Delete))
            timesPressDelete = 0;

        // 重啟遊戲
        if (Input.GetKey (KeyCode.Escape))
        {
            timesPressESC++;
            if (timesPressESC > 150)
            {
                SceneManager.LoadScene (SceneManager.GetActiveScene ().buildIndex);
            }
        }
        else if (Input.GetKeyUp (KeyCode.Escape))
            timesPressESC = 0;
    }

    private void OnDestroy ()
    {
        ArduinoController.Aborting ();
    }

    /*************************************************
     *   General Server Setting
     *************************************************/
    public void SynchronizeClockTime ()
    {
        int sec = DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second;

        // string command = "Base/Clock/" + (DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second) + "/";
        // ArduinoReceivedlMessage.AddMessage (command);
        // PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        // { { PlayerCustomData.COMMAND, command }, { PlayerCustomData.MASTER_TIME, DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second }
        // });

        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        {
            {
                PlayerCustomData.COMMAND,
                    "Base/Clock/" + sec + "/" + PhotonNetwork.LocalPlayer.NickName + "/"
            },
        });
    }

    public void SetBaseSynchronizationIntervalTime ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        {
            {
                PlayerCustomData.COMMAND,
                    "Base/Sync/" + inputSynchronization.text + "/" + PhotonNetwork.LocalPlayer.NickName + "/"
            }
        });
    }

    public void SetBaseTransmissionIntervalTime ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable
        {
            {
                PlayerCustomData.COMMAND,
                    "Base/Trans/" + inputTransmission.text + "/" + PhotonNetwork.LocalPlayer.NickName + "/"
            }
        });
    }
}

[System.Serializable]
public class MessageManager
{
    public Transform window;
    public Transform[] messages;
    private int counter;

    public void Initialize ()
    {
        int count = window.childCount;
        messages = new Transform[count];
        for (int i = 0; i < count; i++)
        {
            messages[i] = window.GetChild (i);
        }
    }

    public void AddMessage (string msg)
    {
        if (msg.Contains ("Clock") || msg.Contains ("DS3231")) return; // 來自時鐘模組
        if (msg.Contains ("S/")) return; // 來自其他Server的訊息
        if (string.IsNullOrEmpty (msg)) return;
        msg = msg.Replace ("Z/", "");
        if (counter < messages.Length)
        {
            messages[counter].gameObject.SetActive (true);
            messages[counter].GetComponentsInChildren<TextMeshProUGUI> () [0].text = ArduinoDashboard.localTime;
            messages[counter].GetComponentsInChildren<TextMeshProUGUI> () [1].text = msg;
            counter++;
        }
        else
        {
            window.GetChild (0).GetComponentsInChildren<TextMeshProUGUI> () [0].text = ArduinoDashboard.localTime;
            window.GetChild (0).GetComponentsInChildren<TextMeshProUGUI> () [1].text = msg;
            window.GetChild (0).SetSiblingIndex (1000);
        }
    }
}