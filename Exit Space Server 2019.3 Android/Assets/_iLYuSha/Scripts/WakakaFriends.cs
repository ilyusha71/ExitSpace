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

    public TextMeshProUGUI textData;

    void Awake ()
    {
        textAgentCode.text = "";
        textServerCode.text = "";
        inputCode.text = PlayerPrefs.GetString ("AgentCode");
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
        Debug.LogWarning ("OnPlayerPropertiesUpdate");
        // if (targetPlayer == PhotonNetwork.LocalPlayer)
        // {
        //     object timeInfo;
        //     object posInfo;
        //     if (changedProps.TryGetValue (PlayerCustomData.LAST_TIME, out timeInfo))
        //     {
        //         if (changedProps.TryGetValue (PlayerCustomData.LAST_POS, out posInfo))
        //         {
        //             textData.text = timeInfo.ToString () + " / " + posInfo.ToString ();
        //         }

        //     }

        // }
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