/***************************************************************************
 * Arduino Controller
 * Arduino控制器
 * Last Updated: 2018/01/10
 * Description:
 * 1. 新的連線狀態 ArduinoStatus
 * 2. 獨立UI監控視窗腳本 ArduinoPanel.cs
 ***************************************************************************/
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using System.Threading;
using UnityEngine;

public enum ArduinoStatus
{
    Unconnected,
    Connecting,
    Connected,
    Aborting,
    Unknown,
}
public static class ArduinoController
{
    // Arduino 連線參數與設定
    private static readonly string PREFS_PORT = "Arduino Port"; // COM10以上無法連接，請通過【裝置管理員】更改COM的編號
    private static readonly string PREFS_RATE = "Arduino Rate";
    private static readonly string[] valueCOM = new string[] { "Unknown", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9" };
    private static readonly int[] valueBaud = new int[] { 0, 9600, 19200, 38400 };
    public static string SerialPort { get; private set; }
    public static int SerialRate { get; private set; }
    public static int Port
    {
        get
        {
            int value = PlayerPrefs.GetInt (PREFS_PORT);
            SerialPort = valueCOM[value];
            return value;
        }
        set
        {
            SerialPort = valueCOM[value];
            PlayerPrefs.SetInt (PREFS_PORT, value);
        }
    }
    public static int Rate
    {
        get
        {
            int value = PlayerPrefs.GetInt (PREFS_RATE);
            SerialRate = valueBaud[value];
            return value;
        }
        set
        {
            SerialRate = valueBaud[value];
            PlayerPrefs.SetInt (PREFS_RATE, value);
        }
    }
    public static void DeletePrefs ()
    {
        PlayerPrefs.DeleteKey (PREFS_PORT);
        PlayerPrefs.DeleteKey (PREFS_RATE);
    }
    public static SerialPort ArduinoConnector;
    private static Thread ArduinoThread;
    public static bool safetyOff = false;
    // Arduino 連線狀態
    public static ArduinoStatus Status = ArduinoStatus.Unconnected;
    public static int timeBoot { get; private set; } //, timeUnload;
    public static bool isRecieving { get; set; }
    public static string Keyword { get; private set; }
    // Arduino 訊息接收
    private static string ArduinoMessages;
    private static string WakakaMessages;
    private static string[] WakakaCommands;
    public static int countCommand { private get; set; }
    public static Queue<string> queueMsg = new Queue<string> ();
    public static Queue<string[]> queueCommand = new Queue<string[]> ();
    public static bool msgQueueCombine = false; // 訊息處理合併

    #region Arduino
    public static void ConnectArduino ()
    {
        if (Status == ArduinoStatus.Unconnected)
        {
            Status = ArduinoStatus.Connecting;
            Keyword = "<color=Yellow>開始連接Arduino</color> <color=white>" + SerialPort + " " + SerialRate + "</color>";
            queueMsg.Enqueue (Keyword);
            Debug.Log (Keyword);
        }
        else if (Status == ArduinoStatus.Connected)
        {
            Keyword = "<color=Yellow>已連接Arduino</color> <color=white>" + ArduinoConnector.PortName + " " + ArduinoConnector.BaudRate + "</color>";
            queueMsg.Enqueue (Keyword);
            Debug.LogWarning (Keyword);
            return;
        }
        else if (Status == ArduinoStatus.Aborting)
        {
            Keyword = "<color=Yellow>請等待Arduino完成中斷後再重新連接</color>";
            queueMsg.Enqueue (Keyword);
            Debug.LogWarning (Keyword);
            return;
        }

        ArduinoConnector = new SerialPort (SerialPort, SerialRate);
        try
        {
            ArduinoConnector.Open ();
            ArduinoThread = new Thread (new ThreadStart (GetArduino));
            ArduinoThread.Start ();
        }
        catch (Exception ex)
        {
            Status = ArduinoStatus.Unconnected;
            Keyword = "<color=#FF8989>Arduino連接失敗</color>：" + ex.Message;
            queueMsg.Enqueue (Keyword);
            Debug.LogWarning (Keyword);
        }
    }

    private static void GetArduino ()
    {
        Keyword = "<color=Yellow>執行緒開始</color> <color=white>GetArduino</color>";
        queueMsg.Enqueue (Keyword);
        Debug.LogWarning (Keyword);
        while (ArduinoThread.IsAlive && Status != ArduinoStatus.Aborting)
        {
            if (ArduinoConnector.IsOpen)
            {
                try
                {
                    ArduinoMessages = ArduinoConnector.ReadLine ();
                    if (string.IsNullOrEmpty (ArduinoMessages)) Debug.LogWarning ("empty");
                    string check = ArduinoMessages.Split ('/') [0];
                    if (check == "ArduinoCallback") queueMsg.Enqueue ("<color=orange>Arduino Callback - 雙向通訊已完成，哇咔咔</color>");
                    else
                    {
                        if (ArduinoMessages.Contains ("ArduinoCallback"))
                            queueMsg.Enqueue ("<color=orange>Arduino Callback - 雙向通訊已完成，哇咔咔卡</color>");
                    }
                    if (check == "Wakaka")
                    {
                        isRecieving = true;
                        WakakaMessages = ArduinoMessages.Replace ("Wakaka/", "");
                        queueMsg.Enqueue (WakakaMessages);

                        WakakaCommands = WakakaMessages.Split ('/');
                        queueCommand.Enqueue (WakakaCommands);
                        // 以下為連續訊息做法
                        // int count = WakakaCommands.Length;
                        // if (count == countCommand)
                        // {
                        //     bool emptyMsg = true;
                        //     for (int i = 0; i < count; i++)
                        //     {
                        //         if (WakakaCommands[i] != "")
                        //             emptyMsg = false;
                        //     }
                        //     if (!emptyMsg)
                        //         queueCommand.Enqueue(WakakaCommands);
                        // }
                    }
                }
                catch (Exception ex)
                {
                    Keyword = "<color=#FF8989>Arduino訊息接收錯誤</color>：" + ex.Message;
                    queueMsg.Enqueue (Keyword);
                    Debug.LogWarning (Keyword);
                }
            }
        }
        Keyword = "<color=Yellow>執行緒結束</color> <color=white>GetArduino</color>";
        queueMsg.Enqueue (Keyword);
        Debug.LogWarning (Keyword);
    }

    public static void DisconnectArduino ()
    {
        if (Status == ArduinoStatus.Connected)
        {
            Status = ArduinoStatus.Aborting;
            Keyword = "<color=Yellow>開始中斷Arduino</color> <color=white>" + ArduinoConnector.PortName + " " + ArduinoConnector.BaudRate + "</color>";
            queueMsg.Enqueue (Keyword);
            Debug.LogWarning (Keyword);
        }
        else if (Status == ArduinoStatus.Unconnected || Status == ArduinoStatus.Connecting)
        {
            Keyword = "<color=Yellow>尚未與Arduino完成連接</color> <color=white>" + SerialPort + " " + SerialRate + "</color>";
            queueMsg.Enqueue (Keyword);
            Debug.LogWarning (Keyword);
            return;
        }

        if (ArduinoThread != null)
        {
            if (ArduinoThread.IsAlive)
                Aborting ();
            else
            {
                Keyword = "<color=#FF8989>執行緒未開啟</color>";
                queueMsg.Enqueue (Keyword);
                Debug.LogWarning (Keyword);
            }
        }
    }

    public static void Aborting ()
    {
        if (Status == ArduinoStatus.Unconnected) return;
        ArduinoConnector.Close ();
        Thread.Sleep (1000);
        ArduinoThread.Abort ();
        Keyword = "<color=Yellow>執行緒正在斷開，當前執行緒狀態</color> <color=white>" + ArduinoThread.IsAlive + "</color>";
        queueMsg.Enqueue (Keyword);
        Debug.Log (Keyword);
    }

    public static void CheckStatus ()
    {
        if (Status == ArduinoStatus.Connecting)
        {
            if (ArduinoThread.IsAlive)
            {
                Status = ArduinoStatus.Connected;
                Keyword = "<color=lime>已成功連接Arduino</color> <color=white>at " + (int) Time.time + "</color>";
                queueMsg.Enqueue (Keyword);
                Debug.Log (Keyword);

                // Arduino Callback
                timeBoot = (int) Time.time;
                ArduinoConnector.WriteLine ("R");
            }
            else
            {
                Status = ArduinoStatus.Connecting;
                Keyword = "<color=yellow>嘗試與Arduino連接中</color>";
                queueMsg.Enqueue (Keyword);
                Debug.LogWarning (Keyword);
            }
        }
        else if (Status == ArduinoStatus.Connected)
        {
            if (!ArduinoConnector.IsOpen)
            {
                Keyword = "<color=#FF8989>SerialPort意外中斷</color>";
                queueMsg.Enqueue (Keyword);
                Debug.LogWarning (Keyword);
            }
            if (!ArduinoThread.IsAlive)
            {
                Keyword = "<color=#FF8989>執行緒意外中斷</color>";
                queueMsg.Enqueue (Keyword);
                Debug.LogWarning (Keyword);
            }
        }
        else if (Status == ArduinoStatus.Aborting)
        {
            if (ArduinoThread.IsAlive)
            {
                Status = ArduinoStatus.Aborting;
                Keyword = "<color=yellow>嘗試與Arduino斷開中</color>";
                queueMsg.Enqueue (Keyword);
                Debug.LogWarning (Keyword);
            }
            else
            {
                Status = ArduinoStatus.Unconnected;
                Keyword = "<color=lime>已成功中斷Arduino</color> <color=white>at " + (int) Time.time + "</color>";
                queueMsg.Enqueue (Keyword);
                Debug.Log (Keyword);

                if (safetyOff)
                    Application.Quit ();
            }
        }
    }

    #endregion

    public static void SafetyOff ()
    {
        safetyOff = true;
        DisconnectArduino ();
    }

    public static void RebootArduino ()
    {
        //DisconnectArduino();
        //msgBox.Keyword("<color=lime>已暫時與Arduino斷開，將於3秒後重新連結</color>");
        //StartCoroutine(Restart());
    }

    static IEnumerator Restart ()
    {
        yield return new WaitForSeconds (3.0f);
        //ConnectArduino();
    }

    static void OnApplicationQuit ()
    {
        //Quit();
    }

    public static void Quit ()
    {
        //Application.Quit();
    }

    static void AutoDestroyCountDown ()
    {
        //countdownDestroy = timeDestroy - Time.time;
        ////textDestroyCD.text = countdownDestroy.ToString("0.0");
        //if (countdownDestroy < 0)
        //{
        //    if (!isAborting)
        //        DisconnectArduino();
        //    if (ArduinoThread != null)
        //    {
        //        if (!ArduinoThread.IsAlive)
        //        {
        //            Debug.LogWarning("Kill Thread");
        //            //Destroy(gameObject);
        //        }
        //    }
        //    else
        //    {
        //        Debug.LogWarning("No Thread");
        //        //Destroy(gameObject);
        //    }
        //}
    }
}