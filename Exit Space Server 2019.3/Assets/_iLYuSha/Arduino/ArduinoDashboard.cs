using System;
using System.Collections.Generic;
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
    public Transform content;
    public PlayableDirector Top2Down, Down2Top;

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
    private TextMeshProUGUI textQuit;
    public Button btnQuit;
    public Button btnTest;
    public Button btnForbbiden;
    public TextMeshProUGUI textTime;
    public int timeRecieve;

    /**********************************************************************
     * 2020/07/26 更新 Arduino Dashboard
     **********************************************************************/

    [Header ("UI - ACM, Arduino Messages")]
    public MessageManager arduinoControlMessage;
    public MessageManager arduinoReceivedlMessage;
    public MessageManager arduinoTransmittedMessage;

    [Header ("UI - Message")]
    public int countCommand;
    public Text textKeyword;
    public Text textMsg;
    private List<string> msgList = new List<string> ();
    private int countMsg;

    void Awake ()
    {
        Instance = this;
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
                    portOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 1f;
                    ArduinoController.Port = index;
                    string Keyword = "<color=lime>重設Serial Port為 </color> <color=white>" + ArduinoController.SerialPort + "</color>";
                    AddNewMsg (Keyword);
                    Debug.Log (Keyword);
                }
                else
                    portOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 0.27f;
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
                    rateOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 1f;
                    ArduinoController.Rate = index;
                    string Keyword = "<color=lime>重設Serial Baud為 </color> <color=white>" + ArduinoController.SerialRate + "</color>";
                    AddNewMsg (Keyword);
                    Debug.Log (Keyword);
                }
                else
                    rateOption[index].GetComponentInChildren<CanvasGroup> ().alpha = 0.36f;
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
        textQuit = btnDisconnect.GetComponentInChildren<TextMeshProUGUI> ();
        // btnQuit.onClick.AddListener (ArduinoController.SafetyOff);
        // btnTest.onClick.AddListener(()=>ArduinoController.ArduinoConnector.WriteLine("R"));
        ArduinoController.countCommand = countCommand;
        panel.SetActive (false);
    }

    void Update ()
    {
        if (Input.GetKeyDown (KeyCode.F9) && Down2Top.time == 0 && Top2Down.time == 0)
            mainPanel.SetActive (!mainPanel.activeSelf);
        content.Translate (new Vector3 (Input.GetAxis ("Horizontal") * -5.0f, 0, 0));

        if (Input.mousePosition.x > Screen.width - 60)
            content.Translate (-500 * Time.deltaTime, 0, 0);
        else if (Input.mousePosition.x < 60)
            content.Translate (500 * Time.deltaTime, 0, 0);

        if ((Input.mousePosition.y < 10 || Input.GetAxis ("Vertical") < 0) && !hasLanding && Down2Top.time == 0)
        {
            Top2Down.Play ();
            hasLanding = true;
        }
        else if ((Input.mousePosition.y > Screen.height - 10 || Input.GetAxis ("Vertical") > 0) && hasLanding && Top2Down.time == 0)
        {
            Down2Top.Play ();
            hasLanding = false;
        }

        if (Input.GetKey (KeyCode.LeftControl) && Input.GetKey (KeyCode.LeftAlt))
        {
            if (Input.GetKeyDown (KeyCode.Slash))
                ArduinoController.ConnectArduino ();
        }
        // textKeyword.text = ArduinoController.Keyword;
        countMsg = ArduinoController.queueMsg.Count;
        for (int i = 0; i < countMsg; i++)
        {
            countBuffer++;
            string msgRx = ArduinoController.queueMsg.Dequeue ();
            if (msgRx.Contains ("Checking"))
                arduinoTransmittedMessage.AddMessage (msgRx + "--- from other [Server]");
            else
                arduinoReceivedlMessage.AddMessage (msgRx);
            AddNewMsg (msgRx.Replace ("Wakaka/", ""));
        }
        if (countBuffer % 30000 == 0)
        {
            countBuffer++;
            GC.Collect ();
            arduinoReceivedlMessage.AddMessage ("[Control]Memory has been deallocated.");
        }
        ArduinoController.CheckStatus ();
        if (ArduinoController.Status == ArduinoStatus.Connected)
            textQuit.text = "Disconnect";
        else if (ArduinoController.Status == ArduinoStatus.Unconnected)
            textQuit.text = "Quit";

        textTime.text = ArduinoController.timeBoot + "\n" + ArduinoController.timeLastReceived + "\n" +
            String.Format ("{0:00}", DateTime.Now.Hour) + ":" +
            String.Format ("{0:00}", DateTime.Now.Minute) + ":" +
            String.Format ("{0:00}", DateTime.Now.Second);;

        // 刪除設定
        if (Input.GetKey (KeyCode.Delete))
        {
            timesPressDelete++;
            if (timesPressDelete > 150)
            {
                ArduinoController.DeletePrefs ();
                portOption[ArduinoController.Port].isOn = true;
                rateOption[ArduinoController.Rate].isOn = true;
                arduinoReceivedlMessage.AddMessage ("[Control]Your settings have been deleted.");
            }
        }
        else if (Input.GetKeyUp (KeyCode.Delete))
            timesPressDelete = 0;

        // 開啟Arduino監控窗
        if (Input.GetKeyDown (KeyCode.F10))
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
        if (msg.Contains ("Clock")) return;
        if (string.IsNullOrEmpty (msg)) return;
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

[System.Serializable]
public class MessageManager
{
    public Transform window;
    public Transform[] messages;
    private int counter;

    public void AddMessage (string msg)
    {
        if (msg.Contains ("Clock")) return; // 來自時鐘模組
        if (msg.Contains ("S/")) return; // 來自其他Server的訊息
        if (string.IsNullOrEmpty (msg)) return;
        msg = msg.Replace ("Z/", "");
        if (counter < messages.Length)
        {
            messages[counter].gameObject.SetActive (true);
            messages[counter].GetComponentsInChildren<TextMeshProUGUI> () [0].text =
                String.Format ("{0:00}", DateTime.Now.Hour) + ":" +
                String.Format ("{0:00}", DateTime.Now.Minute) + ":" +
                String.Format ("{0:00}", DateTime.Now.Second);
            messages[counter].GetComponentsInChildren<TextMeshProUGUI> () [1].text = msg;
            counter++;
        }
        else
        {
            window.GetChild (0).GetComponentsInChildren<TextMeshProUGUI> () [0].text =
                String.Format ("{0:00}", DateTime.Now.Hour) + ":" +
                String.Format ("{0:00}", DateTime.Now.Minute) + ":" +
                String.Format ("{0:00}", DateTime.Now.Second);
            window.GetChild (0).GetComponentsInChildren<TextMeshProUGUI> () [1].text = msg;
            window.GetChild (0).SetSiblingIndex (1000);
        }
    }
}