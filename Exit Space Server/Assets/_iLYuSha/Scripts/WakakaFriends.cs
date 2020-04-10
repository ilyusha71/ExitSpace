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
    public TextMeshProUGUI textPhotonCloudState, textWarning, textServer, textRegionPing;
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
    public GameObject tipSearchRegionServer;
    #endregion

    #region Room
    [Header ("Room Panel")]
    public GameObject panelRoom;
    public TextMeshProUGUI textRoomName;
    public TextMeshProUGUI textMaxPlayers;
    public GameObject roomIsOpen;
    public GameObject roomIsVisible;
    #endregion

    void Awake ()
    {
        inputCode.text = PlayerPrefs.GetString ("InputCode");
    }

    void Update ()
    {
        textPhotonCloudState.text = PhotonNetwork.NetworkClientState.ToString ();
        textRegionPing.text = (PhotonNetwork.IsConnected) ?
            PhotonNetwork.GetPing () + " ms" : "";
    }

    #region UI EVENTS
    /// <summary>
    /// Login Panel
    /// </summary>
    public void OnFastLoginButtonClicked ()
    {
        if (!inputCode.text.Equals (""))
        {
            PlayerPrefs.SetString ("InputCode", inputCode.text);
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
            tipSearchRegionServer.SetActive (true);
            tipSearchRegionServer.GetComponent<TextMeshProUGUI> ().text = "进入目标星系";
        }
    }

    public void OnLogoutButtonClicked ()
    {
        PhotonNetwork.Disconnect ();
    }
    #endregion

    #region PUN CALLBACKS
    // Called to signal that the "low level connection" got established but before the client can call operation on the server.
    public override void OnConnected ()
    {
        // Debug.LogWarning ("OnConnected");
        textWarning.text = NAME_CONNECTED + NAME_ONLINE;
    }
    // Called when the client is connected to the Master Server and ready for matchmaking and other tasks.
    public override void OnConnectedToMaster ()
    {
        // Debug.LogWarning ("OnConnectedToMaster");
        tipSearchRegionServer.SetActive (false);
        // 客戶端新介面
        textServer.text = GetRegionNameText (PhotonNetwork.CloudRegion) + "\n<size=23>" + PhotonNetwork.ServerAddress + "</size>";
        if (!PhotonNetwork.InLobby)
            PhotonNetwork.JoinLobby ();
    }
    // Called after disconnecting from the Photon server. It could be a failure or an explicit disconnect call.
    public override void OnDisconnected (DisconnectCause cause)
    {
        // Debug.LogWarning ("OnDisconnected");
        textWarning.text = NAME_DISCONNECTED + NAME_ONLINE;
        textServer.text = "";
        panelLogin.SetActive (true);
    }

    // Called on entering a lobby on the Master Server. The actual room-list updates will call OnRoomListUpdate.
    public override void OnJoinedLobby ()
    {
        // Debug.LogWarning ("OnJoinedLobby");
        // 直接快速加入新房間
        textWarning.text = "准备" + NAME_JOIN + "未知行星";
        PhotonNetwork.JoinRandomRoom ();
    }

    // Called when the LoadBalancingClient entered a room, no matter if this client created it or simply joined.
    public override void OnJoinedRoom ()
    {
        // Debug.LogWarning ("OnJoinedRoom");
        textWarning.text = NAME_JOINED + PhotonNetwork.CurrentRoom.Name;
        textRoomName.text = PhotonNetwork.CurrentRoom.Name;
        textMaxPlayers.gameObject.SetActive (PhotonNetwork.CurrentRoom.MaxPlayers == 0 ? false : true);
        textMaxPlayers.text = PhotonNetwork.CurrentRoom.MaxPlayers.ToString ();
        roomIsOpen.SetActive (PhotonNetwork.CurrentRoom.IsOpen);
        roomIsVisible.SetActive (PhotonNetwork.CurrentRoom.IsVisible);
        panelLogin.SetActive (false);
        panelRoom.SetActive (true);

        // foreach (Player player in PhotonNetwork.PlayerList)
        // {
        //     if (player != PhotonNetwork.LocalPlayer)
        //     {
        //         GameObject entry = Instantiate (PlayerListEntryPrefab, PlayerListContent);
        //         entry.transform.localScale = Vector3.one;
        //         playerListEntries.Add (player.NickName, entry);
        //         // 以下根據遊戲需求自訂
        //         object customData;
        //         if (player.CustomProperties.TryGetValue ("Data", out customData))
        //         {
        //             PlayerCustomData data = (PlayerCustomData) customData;
        //             entry.GetComponentsInChildren<Image> () [2].enabled = data.Fail;
        //         }
        //     }
        // }
    }
    // Called when a previous OpJoinRoom call failed on the server.
    public override void OnJoinRoomFailed (short returnCode, string message)
    {
        // Debug.LogWarning ("OnJoinRoomFailed");
        if (message == "Game does not exist")
            textWarning.text = NAME_FAILED + NAME_JOIN + "：此行星尚未殖民。";
        else if (message == "Game closed")
            textWarning.text = NAME_FAILED + NAME_JOIN + "：此行星未开放殖民者。";
        else if (message == "Game full")
            textWarning.text = NAME_FAILED + NAME_JOIN + "：此行星殖民者已达上限。";
        else
            textWarning.text = NAME_FAILED + NAME_JOIN + message;
    }
    // Called when a previous OpJoinRandom call failed on the server.
    public override void OnJoinRandomFailed (short returnCode, string message)
    {
        // Debug.LogWarning ("OnJoinRandomFailed");
        textWarning.text = "准备" + NAME_CREATE + "哇咔咔星系";
        RoomOptions options = new RoomOptions
        {
            MaxPlayers = 20,
            IsOpen = true,
            IsVisible = true
        };
        PhotonNetwork.CreateRoom ("哇咔咔星系", options, null);
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