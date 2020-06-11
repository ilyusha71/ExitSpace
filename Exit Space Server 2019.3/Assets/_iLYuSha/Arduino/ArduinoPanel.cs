using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class ArduinoPanel : MonoBehaviour
{
    public static ArduinoPanel Instance { get; private set; }

    [Header ("UI - Arduino Serial Port Setting")]
    public bool DontDestroy;
    public GameObject panel;
    public GameObject listPort;
    public GameObject listRate;
    private Toggle[] portOption;
    private Toggle[] rateOption;
    private float timesPressDelete { get; set; }
    private float timesPressESC { get; set; }
    private int countBuffer; // 緩存計數器

    [Header ("UI - Event")]
    public Button btnConnect;
    public Button btnDisconnect;
    public Button btnQuit;
    public Button btnTest;
    public Button btnForbbiden;
    public Text textTime;
    public int timeRecieve;

    [Header ("UI - Message")]
    public int countCommand;
    public Text textKeyword;
    public Text textMsg;
    private List<string> msgList = new List<string> ();
    private int countMsg;

    void Awake ()
    {
        if (DontDestroy)
            DontDestroyOnLoad (this);
        //SceneManager.LoadScene("Main");

        portOption = listPort.GetComponentsInChildren<Toggle> ();
        int countPort = portOption.Length;
        for (int i = 0; i < countPort; i++)
        {
            int index = i;
            portOption[index].onValueChanged.AddListener (isOn =>
            {
                if (isOn)
                {
                    ArduinoController.Port = index;
                    string Keyword = "<color=lime>重設Serial Port為 </color> <color=white>" + ArduinoController.SerialPort + "</color>";
                    AddNewMsg (Keyword);
                    Debug.Log (Keyword);
                }
            });
        }

        rateOption = listRate.GetComponentsInChildren<Toggle> ();
        int countRate = rateOption.Length;
        for (int i = 0; i < countRate; i++)
        {
            int index = i;
            rateOption[index].onValueChanged.AddListener (isOn =>
            {
                if (isOn)
                {
                    ArduinoController.Rate = index;
                    string Keyword = "<color=lime>重設Serial Baud為 </color> <color=white>" + ArduinoController.SerialRate + "</color>";
                    AddNewMsg (Keyword);
                    Debug.Log (Keyword);
                }
            });
        }

        portOption[ArduinoController.Port].isOn = true;
        rateOption[ArduinoController.Rate].isOn = true;

        btnConnect.onClick.AddListener (ArduinoController.ConnectArduino);
        btnDisconnect.onClick.AddListener (() =>
        {
            if (ArduinoController.Status == ArduinoStatus.Unconnected)
                Application.Quit ();
            else if (ArduinoController.Status == ArduinoStatus.Connected)
                ArduinoController.DisconnectArduino ();
        });
        // btnQuit.onClick.AddListener(ArduinoController.SafetyOff);
        // btnTest.onClick.AddListener(()=>ArduinoController.ArduinoConnector.WriteLine("R"));
        ArduinoController.countCommand = countCommand;
        panel.SetActive (false);
    }

    void Update ()
    {
        if (ArduinoController.isRecieving) timeRecieve = (int) Time.time;
        ArduinoController.isRecieving = false;
        textKeyword.text = ArduinoController.Keyword;
        countMsg = ArduinoController.queueMsg.Count;
        for (int i = 0; i < countMsg; i++) { countBuffer++; AddNewMsg (ArduinoController.queueMsg.Dequeue ().Replace ("Wakaka/", "")); }
        if (countBuffer % 30000 == 0)
        {
            countBuffer++;
            GC.Collect ();
            AddNewMsg ("<color=#FF8989>記憶體已釋放</color>：");
        }
        ArduinoController.CheckStatus ();
        textTime.text = ArduinoController.timeBoot + "\n" + timeRecieve + "\n" + (int) Time.time;

        // 刪除設定
        if (Input.GetKey (KeyCode.Delete))
        {
            timesPressDelete++;
            if (timesPressDelete > 150)
            {
                ArduinoController.DeletePrefs ();
                portOption[ArduinoController.Port].isOn = true;
                rateOption[ArduinoController.Rate].isOn = true;
                string Keyword = "<color=lime>已刪除設定檔</color>";
                textKeyword.text = Keyword;
                AddNewMsg (Keyword);
                Debug.LogWarning (Keyword);
            }
        }
        else if (Input.GetKeyUp (KeyCode.Delete))
            timesPressDelete = 0;

        // 開啟Arduino監控窗
        if (Input.GetKeyDown (KeyCode.F10) || Input.GetKeyDown (KeyCode.KeypadMinus))
            panel.SetActive (!panel.activeSelf);

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

    public void AddNewMsg (string msg)
    {
        if (string.IsNullOrEmpty (msg))
            return;
        byte[] byteArray = System.Text.Encoding.Unicode.GetBytes (msg);
        if (byteArray.Length < 1)
            Debug.LogWarning ("QQ");
        // Arduino傳輸的錯誤，字元的第一個byte錯誤
        try
        {
            if (byteArray[0] == 0)
                return;
        }
        catch (System.Exception e)
        {
            Debug.LogWarning (e.ToString ());
        }
        string str = System.Text.Encoding.Unicode.GetString (byteArray);
        textMsg.text = "";
        if (msgList.Count == 31)
            msgList.RemoveAt (0);
        msgList.Add (str);

        for (int i = 0; i < msgList.Count; i++)
        {
            if (i == 0)
                textMsg.text += ("<color=cyan><i>" + i.ToString ("00") + ": </i></color>" + msgList[i]);
            else
                textMsg.text += ("\n<color=cyan><i>" + i.ToString ("00") + ": </i></color>" + msgList[i]);
        }
    }

    public void Keyword (string msg)
    {
        textKeyword.text = msg;
        AddNewMsg (msg);
    }

    private void OnDestroy ()
    {
        ArduinoController.Aborting ();
    }
}