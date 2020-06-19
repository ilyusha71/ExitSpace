// using System.Collections;
using System.Collections.Generic;
using ExitGames.Client.Photon;
using Photon.Pun;
using Photon.Realtime;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class WakakaFriends : MonoBehaviourPunCallbacks
{
    public Image KGB, CIA;
    private string regionNameText;
    private readonly string NAME_CONNECTED = "已进入";
    private readonly string NAME_DISCONNECTED = "已逃离";
    private readonly string NAME_ONLINE = "光子宇宙";
    private readonly string NAME_CREATE = "殖民";
    private readonly string NAME_JOIN = "前往";
    private readonly string NAME_JOINED = "已登陆";
    private readonly string NAME_FAILED = "无法";

    #region Login
    [Header ("Login Panel")]
    public GameObject panelLogin;
    public TMP_InputField inputCode;
    #endregion

    #region Room
    [Header ("Room Panel")]
    public GameObject panelRoom;
    public TextMeshProUGUI textAgentCode, textServerCode;
    #endregion

    [Header ("Language")]
    public Toggle[] btnLang;
    public TextMeshProUGUI[] textCountdown, textFate, textChance, textBadge;
    public TextMeshProUGUI[] textLogin, textTimeout, textGet, textOpen, textTrial, textTitle;
    private int chooseLang;
    public TextMeshProUGUI textVersion;

    [Header ("Sound")]
    public AudioClip sfxStart;
    public AudioClip sfxDingDong;
    private AudioSource audioSource;

    [Header ("Data")]
    public Image[] checkBadge;
    public TextMeshProUGUI textCountdownMin, textCountdownSec, textFateSec, textChanceMin, textChanceSec;
    public int SERVER_TIME, STAGE_1_TIME, STAGE_2_TIME, STAGE_3_TIME, STAGE_4_TIME;

    [Header ("Window")]
    public GameObject windowLogin, windowTimeout, windowGet, windowOpen, windowTrial, windowTitle;
    public float timer;

    void Awake ()
    {
        textVersion.text = "v" + Application.version;
        Screen.sleepTimeout = SleepTimeout.NeverSleep;
        audioSource = GetComponent<AudioSource> ();

        textAgentCode.text = "";
        textServerCode.text = "";
        inputCode.text = PlayerPrefs.GetString ("AgentCode");

        textCountdownMin.text = "-";
        textCountdownSec.text = "-";
        textFateSec.text = "-";
        textChanceMin.text = "-";
        textChanceSec.text = "-";
        for (int i = 0; i < checkBadge.Length; i++)
        {
            checkBadge[i].enabled = false;
        }

        for (int i = 0; i < btnLang.Length; i++)
        {
            int index = i;
            btnLang[index].onValueChanged.AddListener ((isOn) =>
            {
                if (isOn)
                {
                    audioSource.PlayOneShot (sfxDingDong);
                    chooseLang = index;
                }
                textCountdown[index].enabled = isOn;
                textFate[index].enabled = isOn;
                textChance[index].enabled = isOn;
                textBadge[index].enabled = isOn;
                textLogin[index].enabled = isOn;
                textTimeout[index].enabled = isOn;
                textGet[index].enabled = isOn;
                textOpen[index].enabled = isOn;
                textTrial[index].enabled = isOn;
                textTitle[index].enabled = isOn;
            });
        }
    }
    public void UpdateAPK ()
    {
        Application.OpenURL ("https://github.com/ilyusha71/ExitSpace/blob/master/Exit2019.apk/");
    }

    #region UI EVENTS
    /// <summary>
    /// Login Panel
    /// </summary>
    public void OnFastLoginButtonClicked ()
    {
        if (!inputCode.text.Equals (""))
        {
            PlayerPrefs.SetString ("AgentCode", inputCode.text);
            PhotonNetwork.LocalPlayer.NickName = inputCode.text;
            if (PhotonNetwork.IsConnected)
            {
                if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.exitgames.com")
                    PhotonNetwork.NetworkingClient.ConnectToRegionMaster (PhotonNetwork.PhotonServerSettings.AppSettings.FixedRegion);
                else if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.photonengine.cn")
                    PhotonNetwork.NetworkingClient.ConnectToRegionMaster ("cn");
            }
            else
                PhotonNetwork.ConnectUsingSettings ();
            panelLogin.SetActive (false);
        }
    }
    public void OnLogoutButtonClicked ()
    {
        PhotonNetwork.Disconnect ();
    }
    #endregion

    #region PUN CALLBACKS
    public override void OnConnected () { }
    public override void OnConnectedToMaster ()
    {
        // 客戶端新介面
        if (!PhotonNetwork.InLobby)
            PhotonNetwork.JoinLobby ();
    }
    public override void OnDisconnected (DisconnectCause cause)
    {
        panelLogin.SetActive (true);
    }
    public override void OnJoinedLobby ()
    {
        // 直接快速加入新房間
        PhotonNetwork.JoinRandomRoom ();
    }
    public override void OnJoinedRoom ()
    {
        panelRoom.SetActive (true);
        textAgentCode.text = PhotonNetwork.LocalPlayer.NickName;
        textServerCode.text = PhotonNetwork.MasterClient.NickName;

        if (PhotonNetwork.LocalPlayer.NickName.Contains ("KGB"))
            KGB.enabled = true;
        else if (PhotonNetwork.LocalPlayer.NickName.Contains ("CIA"))
            CIA.enabled = true;
    }
    public override void OnJoinRoomFailed (short returnCode, string message)
    {

    }
    public override void OnJoinRandomFailed (short returnCode, string message)
    {
        RoomOptions options = new RoomOptions
        {
            MaxPlayers = 20,
            IsOpen = true,
            IsVisible = true
        };
        PhotonNetwork.CreateRoom ("哇咔咔星系", options, null);
    }
    public override void OnPlayerPropertiesUpdate (Player targetPlayer, Hashtable changedProps)
    {
        if (targetPlayer == PhotonNetwork.MasterClient)
        {
            // Debug.LogWarning ("On MasterClient Properties Update");
            object data;
            if (changedProps.TryGetValue (PlayerCustomData.LAST_TIME, out data))
            {
                SERVER_TIME =
                    int.Parse (data.ToString ().Split (':') [0]) * 3600 +
                    int.Parse (data.ToString ().Split (':') [1]) * 60 +
                    int.Parse (data.ToString ().Split (':') [2]);

                if (Time.time > timer)
                {
                    windowGet.SetActive (false);
                    windowOpen.SetActive (false);
                    windowTrial.SetActive (false);
                }

                int countdown = 0;
                if (STAGE_1_TIME != 0)
                {
                    if (STAGE_2_TIME != 0)
                    {
                        if (STAGE_3_TIME != 0)
                        {
                            if (STAGE_4_TIME != 0) countdown = Mathf.Max (0, STAGE_1_TIME + PlayerCustomData.STAGE_4_LIMIT - SERVER_TIME);
                            else
                                countdown = Mathf.Max (0, STAGE_1_TIME + PlayerCustomData.STAGE_3_LIMIT - SERVER_TIME);
                        }
                        else
                            countdown = Mathf.Max (0, STAGE_1_TIME + PlayerCustomData.STAGE_2_LIMIT - SERVER_TIME);
                    }
                    else
                        countdown = Mathf.Max (0, STAGE_1_TIME + PlayerCustomData.STAGE_1_LIMIT - SERVER_TIME);
                    int min = Mathf.FloorToInt ((float) countdown / 60.0f);
                    int sec = countdown % 60;
                    textCountdownMin.text = string.Format ("{0:d2}", min);
                    textCountdownSec.text = string.Format ("{0:d2}", sec);
                }
            }
        }
        else if (targetPlayer == PhotonNetwork.LocalPlayer)
        {
            // Debug.LogWarning ("On LocalPlayer Properties Update");
            object data;
            if (changedProps.TryGetValue (PlayerCustomData.LAST_SITE, out data))
            {
                Debug.Log ("Has LAST_POS " + data.ToString ());
                string pos = data.ToString ().Split (':') [0];
                if (ExitSpaceData.IsLocK (pos))
                {
                    audioSource.PlayOneShot (sfxDingDong);
                    windowOpen.SetActive (true);
                    timer = Time.time + 5f;
                }
                if (ExitSpaceData.IsWriter (pos))
                {
                    audioSource.PlayOneShot (sfxDingDong);
                    windowGet.SetActive (true);
                    timer = Time.time + 3f;
                }
            }
            if (changedProps.TryGetValue (PlayerCustomData.LAST_TIME, out data))
            {
                // Debug.Log ("Has LAST_TIME");
                SERVER_TIME =
                    int.Parse (data.ToString ().Split (':') [0]) * 3600 +
                    int.Parse (data.ToString ().Split (':') [1]) * 60 +
                    int.Parse (data.ToString ().Split (':') [2]);
            }
            if (changedProps.TryGetValue (PlayerCustomData.STAGE_1_TIME, out data))
            {
                // Debug.Log ("Has STAGE_1_TIME");
                STAGE_1_TIME =
                    int.Parse (data.ToString ().Split (':') [0]) * 3600 +
                    int.Parse (data.ToString ().Split (':') [1]) * 60 +
                    int.Parse (data.ToString ().Split (':') [2]);
                for (int i = 0; i < checkBadge.Length; i++)
                {
                    checkBadge[i].enabled = false;
                }
                audioSource.clip = sfxStart;
                audioSource.Play ();
                windowLogin.SetActive (false);
            }
            if (changedProps.TryGetValue (PlayerCustomData.STAGE_2_TIME, out data))
            {
                // Debug.Log ("Has STAGE_2_TIME");
                STAGE_2_TIME =
                    int.Parse (data.ToString ().Split (':') [0]) * 3600 +
                    int.Parse (data.ToString ().Split (':') [1]) * 60 +
                    int.Parse (data.ToString ().Split (':') [2]);
            }
            for (int i = 0; i < checkBadge.Length; i++)
            {
                if (changedProps.TryGetValue (PlayerCustomData.BADGE[i], out data))
                {
                    checkBadge[i].enabled = (bool) data;
                    Debug.LogWarning (i + " is " + (bool) data);
                }
            }
        }
    }
    public override void OnMasterClientSwitched (Player newMasterClient)
    {
        textServerCode.text = newMasterClient.NickName;
    }
    #endregion

    #region Advanced Method
    private string GetRegionNameText (string region)
    {
        if (region.Contains ("/*"))
            region = region.Replace ("/*", "");
        switch (region)
        {
            case "eu":
                return "欧洲地区";
            case "us":
                return "美国东部";
            case "usw":
                return "美国西部";
            case "cae":
                return "加拿大东部";
            case "asia":
                return "亚洲地区";
            case "jp":
                return "日本";
            case "au":
                return "澳大利亚";
            case "sa":
                return "南美地区";
            case "in":
                return "印度";
            case "ru":
                return "俄罗斯";
            case "rue":
                return "远东地区";
            case "kr":
                return "南韩";
            case "za":
                return "南非";
            case "cn":
                return "中国";
            default:
                return "未知星系";
        }
    }
    #endregion
}