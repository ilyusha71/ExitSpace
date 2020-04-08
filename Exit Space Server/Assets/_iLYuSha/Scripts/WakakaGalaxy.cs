using System.Collections;
using System.Collections.Generic;
using ExitGames.Client.Photon;
using Photon.Pun;
using Photon.Realtime;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class WakakaGalaxy : MonoBehaviourPunCallbacks
{
    public TextMeshProUGUI textPhotonCloudState, textWarning, textServer, textRegionPing;
    private string regionNameText;
    private readonly string NAME_CONNECTED = "已进入";
    private readonly string NAME_DISCONNECTED = "已逃离";
    private readonly string NAME_ONLINE = "光子宇宙";
    private readonly string NAME_SERVER = "星系";
    private readonly string NAME_LOBBY = "行星系";
    private readonly string NAME_CREATE = "殖民";
    private readonly string NAME_CREATED = "已殖民";
    private readonly string NAME_JOIN = "前往";
    private readonly string NAME_JOINED = "已进入";
    private readonly string NAME_ROOM = "行星";
    private readonly string NAME_LEFT = "撤离";
    private readonly string NAME_FAILED = "失败";
    #region Login
    public GameObject panelLogin;
    public TMP_InputField inputCode;
    public GameObject tipSearchRegionServer;
    public Transform groupRegion;
    private List<Button> listRegionButton = new List<Button> ();
    private List<Region> listRegion;
    private bool showRegion = false;
    private readonly Color32 colorVeryStrong = new Color32 (0, 255, 30, 255);
    private readonly Color32 colorStrong = new Color32 (209, 255, 69, 255);
    private readonly Color32 colorMedium = new Color32 (255, 196, 0, 255);
    private readonly Color32 colorWeak = new Color32 (255, 100, 0, 255);
    private readonly Color32 colorVeryWeak = new Color32 (255, 59, 59, 255);
    #endregion

    #region Lobby
    public GameObject panelLobby;
    private int roomIndex = 1;
    private string roomName = "";
    #endregion

    #region Room
    public GameObject panelRoom;
    public TextMeshProUGUI textRoomName;
    #endregion

    void Awake ()
    {
        Button[] btns = groupRegion.GetComponentsInChildren<Button> ();
        for (int i = 0; i < btns.Length; i++)
        {
            listRegionButton.Add (btns[i]);
            btns[i].gameObject.SetActive (false);
        }
        inputCode.text = PlayerPrefs.GetString ("InputCode");
        Resources.UnloadUnusedAssets ();
    }

    void New ()
    {
        // PhotonNetwork.NetworkingClien
    }

    void Update ()
    {
        textPhotonCloudState.text = PhotonNetwork.NetworkClientState.ToString ();
        textRegionPing.text = (PhotonNetwork.IsConnected) ?
            PhotonNetwork.GetPing () + " ms" : "";
        if (showRegion)
        {
            showRegion = false;
            tipSearchRegionServer.SetActive (false);
            textWarning.text = "区域" + NAME_SERVER + "已测试完成";
            int countRegion = listRegion.Count;
            while (listRegionButton.Count < countRegion)
            {
                Button btn = Instantiate (listRegionButton[0], groupRegion);
                listRegionButton.Add (btn);
            }
            for (int i = 0; i < countRegion; i++)
            {
                listRegionButton[i].gameObject.SetActive (true);
                int ping = listRegion[i].Ping;
                listRegionButton[i].GetComponentsInChildren<TextMeshProUGUI> () [0].text =
                    GetRegionNameText (listRegion[i].Code);
                listRegionButton[i].GetComponentsInChildren<TextMeshProUGUI> () [1].text =
                    ping + " ms";
                Image pingDisplay = listRegionButton[i].GetComponentsInChildren<Image> () [2];
                if (ping < 70)
                {
                    pingDisplay.color = colorVeryStrong;
                    pingDisplay.fillAmount = 1.0f;
                }
                else if (ping < 100)
                {
                    pingDisplay.color = colorStrong;
                    pingDisplay.fillAmount = 0.8f;
                }
                else if (ping < 170)
                {
                    pingDisplay.color = colorMedium;
                    pingDisplay.fillAmount = 0.6f;
                }
                else if (ping < 250)
                {
                    pingDisplay.color = colorWeak;
                    pingDisplay.fillAmount = 0.4f;
                }
                else
                {
                    pingDisplay.color = colorVeryWeak;
                    pingDisplay.fillAmount = 0.2f;
                }
                int indexRegion = i;
                listRegionButton[indexRegion].onClick.AddListener (() =>
                {
                    OnRegionButtonClicked (listRegion[indexRegion].Code);
                });
            }
        }
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
    public void OnInternationServerButtonClicked ()
    {
        textWarning.text = "寻找区域星系列表";
        tipSearchRegionServer.SetActive (true);
        tipSearchRegionServer.GetComponent<TextMeshProUGUI> ().text = "正在建立星系传送门";
        PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime = "6790855d-98ce-4acd-b64f-688c4a354ccf"; // TODO: replace with your own AppId
        PhotonNetwork.NetworkingClient.AppId = PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime;
        // 如果使用ConnectUsingSettings連線會自動在遊戲版本加入PUN版本的後綴。
        // 為確保兩種連線方式的NetworkingClient.AppVersion相同，必須加入下面這行
        PhotonNetwork.GameVersion = PhotonNetwork.PhotonServerSettings.AppSettings.AppVersion;
        PhotonNetwork.NetworkingClient.NameServerHost = "ns.exitgames.com";
        CloseRegion ();
        PhotonNetwork.NetworkingClient.ConnectToNameServer ();
    }
    public void OnChinaServerButtonClicked ()
    {
        tipSearchRegionServer.SetActive (true);
        tipSearchRegionServer.GetComponent<TextMeshProUGUI> ().text = "正在建立星系传送门";
        PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime = "8d3d0446-b335-47c1-ab69-7aa9e6878c10"; // TODO: replace with your own AppId
        PhotonNetwork.NetworkingClient.AppId = PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime;
        PhotonNetwork.GameVersion = PhotonNetwork.PhotonServerSettings.AppSettings.AppVersion;
        PhotonNetwork.NetworkingClient.NameServerHost = "ns.photonengine.cn";
        CloseRegion ();
        PhotonNetwork.NetworkingClient.ConnectToNameServer ();
    }
    public void OnRegionButtonClicked (string region)
    {
        if (!inputCode.text.Equals (""))
        {
            PlayerPrefs.SetString ("InputCode", inputCode.text);
            PhotonNetwork.LocalPlayer.NickName = inputCode.text;
            PhotonNetwork.NetworkingClient.ConnectToRegionMaster (region);
            panelLogin.SetActive (false);
            tipSearchRegionServer.SetActive (true);
            tipSearchRegionServer.GetComponent<TextMeshProUGUI> ().text = "进入目标星系";
        }
        else
        {
            Debug.LogError ("input Code is invalid.");
        }
    }

    /// <summary>
    /// Lobby Panel
    /// </summary>
    public void OnChangeRegionButtonClicked ()
    {
        panelLogin.SetActive (true);
        panelLobby.SetActive (false);
    }
    public void OnLogoutButtonClicked ()
    {
        PhotonNetwork.Disconnect ();
    }
    public void OnCreateRoomButtonClicked ()
    {
        roomName = "逃生门" + roomIndex + "号星";
        textWarning.text = NAME_CREATE + roomName;
        roomName = (roomName.Equals (string.Empty)) ? "Room " + Random.Range (1000, 10000) : roomName;

        byte maxPlayers;
        byte.TryParse ("10", out maxPlayers);
        maxPlayers = (byte) Mathf.Clamp (maxPlayers, 2, 8);

        RoomOptions options = new RoomOptions { MaxPlayers = maxPlayers };

        PhotonNetwork.CreateRoom (roomName, options, null);
    }
    public void OnJoinRoomButtonClicked ()
    {
        roomName = "逃生门" + roomIndex + "号星";
        textWarning.text = NAME_JOIN + roomName;
        PhotonNetwork.JoinRoom (roomName);
    }
    public void OnJoinOrCreateRoomButtonClicked ()
    {
        roomName = "逃生门" + roomIndex + "号星";
        textWarning.text = NAME_JOIN + roomName;
        PhotonNetwork.JoinOrCreateRoom (roomName, null, null, null);
    }
    public void OnJoinRandomRoomButtonClicked ()
    {
        textWarning.text = NAME_JOIN + "未知行星";
        PhotonNetwork.JoinRandomRoom ();
    }

    /// <summary>
    /// Room Panel
    /// </summary>
    public void OnLeftRoomButtonClicked ()
    {
        textWarning.text = NAME_LEFT + PhotonNetwork.CurrentRoom.Name;
        PhotonNetwork.LeaveRoom ();
    }
    #endregion

    #region PUN CALLBACKS
    /// <summary>
    /// IConnectionCallbacks Interface 連線回傳介面
    /// </summary>
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
        panelLobby.SetActive (true);
        textWarning.text = NAME_CONNECTED + "主" + NAME_SERVER;
        textServer.text = GetRegionNameText (PhotonNetwork.CloudRegion) + "\n<size=23>" + PhotonNetwork.ServerAddress + "</size>";
    }
    // Called after disconnecting from the Photon server. It could be a failure or an explicit disconnect call.
    public override void OnDisconnected (DisconnectCause cause)
    {
        // Debug.LogWarning ("OnDisconnected");
        textWarning.text = NAME_DISCONNECTED + NAME_ONLINE;
        textServer.text = "";
        panelLogin.SetActive (true);
        panelLobby.SetActive (false);
        CloseRegion ();
    }
    // Called when the Name Server provided a list of regions for your title. 
    public override void OnRegionListReceived (RegionHandler regionHandler)
    {
        // Debug.LogWarning ("OnRegionListReceived");
        textWarning.text = "已收到区域" + NAME_SERVER + "列表";
        if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.exitgames.com")
            textServer.text = "星际空间";
        else if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.photonengine.cn")
            textServer.text = "本星系团";
        regionHandler.PingMinimumOfRegions (this.OnRegionPingCompleted, null);
    }
    private void OnRegionPingCompleted (RegionHandler regionHandler)
    {
        listRegion = new List<Region> (regionHandler.EnabledRegions);
        Debug.Log ("OnRegionPingCompleted " + regionHandler.BestRegion);
        Debug.Log ("RegionPingSummary: " + regionHandler.SummaryToCache);
        showRegion = true;
    }
    // Called when your Custom Authentication service responds with additional data.
    public override void OnCustomAuthenticationResponse (Dictionary<string, object> data)
    {
        Debug.LogWarning ("OnCustomAuthenticationResponse");
    }
    // Called when the custom authentication failed. Followed by disconnect!
    public override void OnCustomAuthenticationFailed (string debugMessage)
    {
        Debug.LogError ("OnCustomAuthenticationFailed" + debugMessage);
    }

    /// <summary>
    /// ILobbyCallbacks Interface 大廳回傳介面
    /// </summary>
    // Called on entering a lobby on the Master Server. The actual room-list updates will call OnRoomListUpdate.
    public override void OnJoinedLobby ()
    {
        Debug.LogWarning ("OnJoinedLobby");
    }
    // Called after leaving a lobby.
    public override void OnLeftLobby ()
    {
        Debug.LogWarning ("OnLeftLobby");
    }
    // Called for any update of the room-listing while in a lobby (InLobby) on the Master Server.
    public override void OnRoomListUpdate (List<RoomInfo> roomList)
    {
        // Debug.LogWarning ("OnRoomListUpdate");
        textWarning.text = NAME_ROOM + "列表已更新";
    }
    // Called when the Master Server sent an update for the Lobby Statistics.
    public override void OnLobbyStatisticsUpdate (List<TypedLobbyInfo> lobbyStatistics)
    {
        Debug.LogWarning ("OnLobbyStatisticsUpdate");
    }

    /// <summary>
    /// IMatchmakingCallbacks Interface 匹配回傳介面
    /// </summary>
    // Called when the server sent the response to a FindFriends request.
    public override void OnFriendListUpdate (List<FriendInfo> friendList) { }
    // Called when this client created a room and entered it. OnJoinedRoom() will be called as well.
    public override void OnCreatedRoom ()
    {
        // Debug.LogWarning ("OnCreatedRoom");
        textWarning.text = NAME_CREATED + PhotonNetwork.CurrentRoom.Name;
    }
    // Called when the server couldn't create a room (OpCreateRoom failed).
    public override void OnCreateRoomFailed (short returnCode, string message)
    {
        // Debug.LogWarning ("OnCreateRoomFailed");
        textWarning.text = NAME_CREATED + roomName + NAME_FAILED;
        roomIndex++;
    }
    // Called when the LoadBalancingClient entered a room, no matter if this client created it or simply joined.
    public override void OnJoinedRoom ()
    {
        // Debug.LogWarning ("OnJoinedRoom");
        textWarning.text = NAME_JOINED + PhotonNetwork.CurrentRoom.Name;
        textRoomName.text = PhotonNetwork.CurrentRoom.Name;
        roomName = PhotonNetwork.CurrentRoom.Name;
        panelLobby.SetActive (false);
        panelRoom.SetActive (true);
    }
    // Called when a previous OpJoinRoom call failed on the server.
    public override void OnJoinRoomFailed (short returnCode, string message)
    {
        // Debug.LogWarning ("OnJoinRoomFailed");
        textWarning.text = NAME_JOIN + roomName + NAME_FAILED;
    }
    // Called when a previous OpJoinRandom call failed on the server.
    public override void OnJoinRandomFailed (short returnCode, string message)
    {
        // Debug.LogWarning ("OnJoinRandomFailed");
        textWarning.text = NAME_JOIN + "未知行星" + NAME_FAILED;
        OnCreateRoomButtonClicked ();
    }
    // Called when the local user / client left a room, so the game 's logic can clean up it's internal state.
    public override void OnLeftRoom ()
    {
        // Debug.LogWarning ("OnLeftRoom");
        textWarning.text = NAME_LEFT + roomName;
        panelLobby.SetActive (true);
        panelRoom.SetActive (false);
    }

    #endregion

    void CloseRegion ()
    {
        for (int i = 0; i < listRegionButton.Count; i++)
        {
            listRegionButton[i].gameObject.SetActive (false);
        }
    }

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
}