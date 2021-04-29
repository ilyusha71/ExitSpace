// using System.Collections;
using System;
using System.Collections.Generic;
using ExitGames.Client.Photon;
using Photon.Pun;
using Photon.Realtime;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using Random = UnityEngine.Random;

public class WakakaGalaxy : MonoBehaviourPunCallbacks
{
    public TextMeshProUGUI textVersionPUN, textTime, textPhotonCloudState, textServer, textRegionPing, textWarning;
    private string regionNameText;
    private readonly string NAME_CONNECTED = "已进入";
    private readonly string NAME_DISCONNECTED = "已逃离";
    private readonly string NAME_ONLINE = "光子宇宙";
    private readonly string NAME_SERVER = "星系";
    // private readonly string NAME_LOBBY = "行星系";
    private readonly string NAME_CREATE = "殖民";
    private readonly string NAME_CREATED = "已殖民";
    private readonly string NAME_JOIN = "前往";
    private readonly string NAME_JOINED = "已登陆";
    private readonly string NAME_ROOM = "行星";
    private readonly string NAME_LEAVE = "撤离";
    private readonly string NAME_LEFT = "已撤离";
    private readonly string NAME_FAILED = "无法";

    #region Login
    [Header ("Login Panel")]
    public GameObject panelLogin;
    public TMP_InputField inputCode;
    public Button btnInternational, btnChina;
    public Animation animConnecting;
    public GameObject tipSearchRegionServer;
    public Transform groupRegion;
    public Sprite[] iconFlags;
    private List<Button> listRegionButton = new List<Button> ();
    private List<Region> listRegion;
    private bool showRegion = false;
    private readonly Color32 colorVeryStrong = new Color32 (0, 255, 30, 255);
    private readonly Color32 colorStrong = new Color32 (209, 255, 69, 255);
    private readonly Color32 colorMedium = new Color32 (255, 196, 0, 255);
    private readonly Color32 colorWeak = new Color32 (255, 100, 0, 255);
    private readonly Color32 colorVeryWeak = new Color32 (255, 59, 59, 255);
    private int numServices; // 1 = dont, 0 = do it
    private float timeCheckViolation;
    #endregion

    #region Lobby
    [Header ("Lobby Panel")]
    public GameObject panelLobby;
    public GameObject windowCommand;
    public GameObject windowOption;
    public TMP_InputField inputRoomName;
    public TMP_InputField inputMaxPlayers;
    public TextMeshProUGUI tipMaxPlayers;
    public Toggle toggleIsOpen;
    public Toggle toggleIsVisible;
    public Button settingButton;
    [Header ("Lobby Panel - List of Room")]
    public RectTransform roomListContent;
    public GameObject RoomListEntryPrefab;
    public GameObject btnRefreshRoomList;
    public Sprite[] iconPlanets;
    private Dictionary<string, RoomInfo> cachedRoomList = new Dictionary<string, RoomInfo> ();
    private Dictionary<string, GameObject> roomListEntries = new Dictionary<string, GameObject> ();
    #endregion

    #region Room
    [Header ("Room Panel")]
    public GameObject panelRoom;
    public TextMeshProUGUI textRoomName;
    public TextMeshProUGUI textMaxPlayers;
    public GameObject roomIsOpen;
    public GameObject roomIsVisible;
    [Header ("Room Panel - List of Player")]
    public Transform PlayerListContent;
    public GameObject PlayerListEntryPrefab;
    private Dictionary<string, GameObject> playerListEntries = new Dictionary<string, GameObject> ();
    #endregion

    void Awake ()
    {
        PhotonNetwork.GameVersion = PhotonNetwork.PhotonServerSettings.AppSettings.AppVersion;
        textVersionPUN.text = "PUN " + PhotonNetwork.GameVersion;
        numServices = PlayerPrefs.GetInt ("ServiceMode");
        if (numServices == 0) inputCode.text = PlayerPrefs.GetString ("InputCode");
        else inputCode.text = "";
        Button[] btns = groupRegion.GetComponentsInChildren<Button> ();
        for (int i = 0; i < btns.Length; i++)
        {
            listRegionButton.Add (btns[i]);
            btns[i].gameObject.SetActive (false);
        }
    }

    void Update ()
    {
        textTime.text = ArduinoDashboard.localTime;
        textPhotonCloudState.text = PhotonNetwork.NetworkClientState.ToString ();
        textRegionPing.text = PhotonNetwork.GetPing ().ToString ();

        if (showRegion)
        {
            showRegion = false;
            btnChina.interactable = true;
            btnInternational.interactable = true;
            textWarning.text = "区域" + NAME_SERVER + "已测试完成";
            for (int i = 0; i < listRegion.Count; i++)
            {
                int ping = listRegion[i].Ping;
                Image pingDisplay = listRegionButton[i].GetComponentsInChildren<Image> () [3];
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
                TextMeshProUGUI textPing = listRegionButton[i].GetComponentsInChildren<TextMeshProUGUI> () [1];
                textPing.text = ping.ToString ();
                textPing.color = pingDisplay.color;
            }
        }
        if (numServices > 0)
        {
            if (PhotonNetwork.InRoom)
            {
                if (PhotonNetwork.CurrentRoom.MaxPlayers != 0 && PhotonNetwork.CurrentRoom.MaxPlayers < 20)
                {
                    if (Time.time > timeCheckViolation)
                    {
                        if (PhotonNetwork.CurrentRoom.PlayerCount < 3) timeCheckViolation += 60;
                        else if (PhotonNetwork.CurrentRoom.MaxPlayers - PhotonNetwork.CurrentRoom.PlayerCount < 3) timeCheckViolation += 60;
                        else
                        {
                            int rand = Random.Range (0, 100);
                            Debug.LogWarning (rand);
                            if (rand > 93)
                                PhotonNetwork.Disconnect ();
                            else if (rand > 85)
                                PhotonNetwork.LeaveRoom ();
                            else if (rand > 70)
                            {
                                if (PhotonNetwork.MasterClient.NickName != "iLYuSha")
                                {
                                    Player p = PhotonNetwork.PlayerList[Random.Range (0, PhotonNetwork.PlayerList.Length - 1)];
                                    PhotonNetwork.SetMasterClient (p);
                                }
                            }
                            else if (rand < 50)
                            {
                                if (PhotonNetwork.MasterClient.NickName != "iLYuSha")
                                    PhotonNetwork.SetMasterClient (PhotonNetwork.LocalPlayer);
                            }
                            timeCheckViolation += Random.Range (60, 240);
                        }
                    }
                }
            }
        }
    }

    #region UI EVENTS
    /// <summary>
    /// Login Panel
    /// </summary>
    public void OpenServiceMode ()
    {
        numServices = 1;
        PlayerPrefs.SetInt ("ServiceMode", numServices);
    }
    public void OpenDevMode ()
    {
        numServices = 0;
        PlayerPrefs.DeleteKey ("ServiceMode");
    }
    public void OnFastLoginButtonClicked ()
    {
        if (!inputCode.text.Equals (""))
        {
            if (numServices == 0)
            {
                PlayerPrefs.SetString ("InputCode", inputCode.text);
                if (inputCode.text.Equals ("Knights") || inputCode.text.Equals ("Agents"))
                    OpenServiceMode ();
            }
            else
            {
                if (!inputCode.text.Equals ("Knights") && !inputCode.text.Equals ("Agents"))
                {
                    string currentSceneName = SceneManager.GetActiveScene ().name;
                    SceneManager.LoadScene (currentSceneName);
                    return;
                }
            }
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
        OnLogoutButtonClicked ();
        animConnecting.Play ("Interstellar");
        textWarning.text = "寻找区域星系列表";
        tipSearchRegionServer.SetActive (true);
        tipSearchRegionServer.GetComponent<TextMeshProUGUI> ().text = "正在建立星系传送门";
        PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime = "84f4bc51-fb6f-4613-b4ca-1fedcb3e1151"; // TODO: replace with your own AppId
        PhotonNetwork.NetworkingClient.AppId = PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime;
        PhotonNetwork.NetworkingClient.NameServerHost = "ns.exitgames.com";
        PhotonNetwork.NetworkingClient.ConnectToNameServer ();
    }
    public void OnChinaServerButtonClicked ()
    {
        OnLogoutButtonClicked ();
        animConnecting.Play ("China");
        tipSearchRegionServer.SetActive (true);
        tipSearchRegionServer.GetComponent<TextMeshProUGUI> ().text = "正在建立星系传送门";
        PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime = "8d3d0446-b335-47c1-ab69-7aa9e6878c10"; // TODO: replace with your own AppId
        PhotonNetwork.NetworkingClient.AppId = PhotonNetwork.PhotonServerSettings.AppSettings.AppIdRealtime;
        PhotonNetwork.NetworkingClient.NameServerHost = "ns.photonengine.cn";
        PhotonNetwork.NetworkingClient.ConnectToNameServer ();
    }
    public void OnRegionButtonClicked (string region)
    {
        if (!inputCode.text.Equals (""))
        {
            if (numServices == 0)
            {
                PlayerPrefs.SetString ("InputCode", inputCode.text);
                if (inputCode.text.Equals ("Knights") || inputCode.text.Equals ("Agents"))
                    OpenServiceMode ();
            }
            else
            {
                if (!inputCode.text.Equals ("Knights") && !inputCode.text.Equals ("Agents"))
                {
                    string currentSceneName = SceneManager.GetActiveScene ().name;
                    SceneManager.LoadScene (currentSceneName);
                    return;
                }
            }
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
        if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.exitgames.com")
            OnInternationServerButtonClicked ();
        else if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.photonengine.cn")
            OnChinaServerButtonClicked ();
    }
    public void OnLogoutButtonClicked ()
    {
        animConnecting.Play ("Initial");
        PhotonNetwork.Disconnect ();
    }
    public void OnMaxPlayerChange (string count)
    {
        int valueMaxPlayer;
        int.TryParse (count, out valueMaxPlayer);
        if (valueMaxPlayer < 0) // 0 代表无限制
        {
            tipMaxPlayers.text = "殖民者不足";
            settingButton.gameObject.SetActive (false);
        }
        else if (valueMaxPlayer > 20)
        {
            tipMaxPlayers.text = "殖民者过量";
            settingButton.gameObject.SetActive (false);
        }
        else
        {
            tipMaxPlayers.text = "";
            settingButton.gameObject.SetActive (true);
        }
    }
    public void OnCreateRoomButtonClicked ()
    {
        string roomName = !inputRoomName.text.Equals (string.Empty) ? inputRoomName.text : "Wakaka" + Random.Range (1000, 10000);
        byte maxPlayers;
        byte.TryParse (inputMaxPlayers.text, out maxPlayers);
        if (maxPlayers < 0) // 0 代表无限制
            textWarning.text = "殖民者不足";
        else if (maxPlayers > 20)
            textWarning.text = "殖民者过量";
        else
        {
            textWarning.text = "准备" + NAME_CREATE + roomName;
            RoomOptions options = new RoomOptions
            {
                MaxPlayers = maxPlayers,
                IsOpen = toggleIsOpen.isOn,
                IsVisible = toggleIsVisible.isOn
            };
            PhotonNetwork.CreateRoom (roomName, options, null);
        }
    }
    public void OnJoinRoomButtonClicked ()
    {
        string roomName = !inputRoomName.text.Equals (string.Empty) ? inputRoomName.text : "Wakaka" + Random.Range (1000, 10000);
        textWarning.text = "准备" + NAME_JOIN + roomName;
        PhotonNetwork.JoinRoom (roomName);
    }
    public void OnJoinOrCreateRoomButtonClicked ()
    {
        string roomName;
        if (numServices == 0)
            roomName = !inputRoomName.text.Equals (string.Empty) ? inputRoomName.text : "Wakaka" + Random.Range (1000, 10000);
        else
        {
            if (inputRoomName.text.Equals ("Wakaka"))
                roomName = "Wakaka" + Random.Range (1000, 10000);
            else if (inputRoomName.text.Equals ("Mission"))
                roomName = "Mission" + Random.Range (1000, 10000);
            else
            {
                string currentSceneName = SceneManager.GetActiveScene ().name;
                SceneManager.LoadScene (currentSceneName);
                return;
            }

            byte mp;
            byte.TryParse (inputMaxPlayers.text, out mp);
            if (mp <= 0) // 0 代表无限制
            {
                string currentSceneName = SceneManager.GetActiveScene ().name;
                SceneManager.LoadScene (currentSceneName);
                return;
            }
        }
        byte maxPlayers;
        byte.TryParse (inputMaxPlayers.text, out maxPlayers);
        if (maxPlayers < 0) // 0 代表无限制
            textWarning.text = "殖民者不足";
        else if (maxPlayers > 20)
            textWarning.text = "殖民者过量";
        else
        {
            textWarning.text = "准备" + NAME_JOIN + roomName;
            RoomOptions options = new RoomOptions
            {
                MaxPlayers = maxPlayers,
                IsOpen = toggleIsOpen.isOn,
                IsVisible = toggleIsVisible.isOn
            };
            PhotonNetwork.JoinOrCreateRoom (roomName, options, null);
        }
    }
    public void OnJoinRandomRoomButtonClicked ()
    {
        textWarning.text = "准备" + NAME_JOIN + "未知行星";
        PhotonNetwork.JoinRandomRoom ();
    }
    public void OnRefreshRoomInfoButtonClicked ()
    {
        PhotonNetwork.JoinLobby ();
    }

    /// <summary>
    /// Room Panel
    /// </summary>
    public void OnPlusMaxPlayerButtonClicked ()
    {
        if (PhotonNetwork.CurrentRoom.MaxPlayers == 0) return;
        PhotonNetwork.CurrentRoom.MaxPlayers = (byte) Mathf.Clamp (++PhotonNetwork.CurrentRoom.MaxPlayers, 4, 11);
    }
    public void OnMinusMaxPlayerButtonClicked ()
    {
        if (PhotonNetwork.CurrentRoom.MaxPlayers == 0) return;
        PhotonNetwork.CurrentRoom.MaxPlayers = (byte) Mathf.Clamp (--PhotonNetwork.CurrentRoom.MaxPlayers, PhotonNetwork.CurrentRoom.PlayerCount, 11);
        PhotonNetwork.CurrentRoom.MaxPlayers = (byte) Mathf.Clamp (PhotonNetwork.CurrentRoom.MaxPlayers, 4, 11);
    }
    public void OnLeaveRoomButtonClicked ()
    {
        // textWarning.text = "准备" + NAME_LEAVE + PhotonNetwork.CurrentRoom.Name;
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
        textWarning.text = NAME_CONNECTED + NAME_ONLINE;
    }
    // Called when the client is connected to the Master Server and ready for matchmaking and other tasks.
    public override void OnConnectedToMaster ()
    {
        Debug.LogWarning ("OnConnectedToMaster");
        Debug.Log ("Region: " + PhotonNetwork.CloudRegion);
        Debug.Log ("IP: " + PhotonNetwork.ServerAddress);
        Debug.Log ("Lobby: " + PhotonNetwork.InLobby);
        textVersionPUN.text = "PUN " + PhotonNetwork.GameVersion;

        tipSearchRegionServer.SetActive (false);
        panelLobby.SetActive (true);
        windowCommand.SetActive (true);
        windowOption.SetActive (false);
        textWarning.text = NAME_CONNECTED + "主" + NAME_SERVER;
        textServer.text = GetRegionNameText (PhotonNetwork.CloudRegion) + "\n<size=23>" + PhotonNetwork.ServerAddress + "</size>";
        if (!PhotonNetwork.InLobby)
            btnRefreshRoomList.SetActive (true);
    }
    // Called after disconnecting from the Photon server. It could be a failure or an explicit disconnect call.
    public override void OnDisconnected (DisconnectCause cause)
    {
        Debug.LogWarning ("OnDisconnected: " + cause.ToString ());
        textWarning.text = NAME_DISCONNECTED + NAME_ONLINE;
        textServer.text = "";
        panelLogin.SetActive (true);
        panelLobby.SetActive (false);
        CloseRegion ();
        ClearRoomListView();
    }
    // Called when the Name Server provided a list of regions for your title. 
    public override void OnRegionListReceived (RegionHandler regionHandler)
    {
        // Debug.LogWarning ("OnRegionListReceived");
        btnInternational.interactable = false;
        btnChina.interactable = false;
        tipSearchRegionServer.SetActive (false);
        textWarning.text = "已收到区域" + NAME_SERVER + "列表";
        if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.exitgames.com")
            textServer.text = "星际空间";
        else if (PhotonNetwork.NetworkingClient.NameServerHost == "ns.photonengine.cn")
            textServer.text = "本星系团";
        if (regionHandler.PingMinimumOfRegions (this.OnRegionPingCompleted, null))
        {
            listRegion = new List<Region> (regionHandler.EnabledRegions);
            int countRegion = listRegion.Count;
            while (listRegionButton.Count < countRegion)
            {
                Button btn = Instantiate (listRegionButton[0], groupRegion);
                listRegionButton.Add (btn);
            }
            for (int i = 0; i < countRegion; i++)
            {
                listRegionButton[i].gameObject.SetActive (true);
                listRegionButton[i].GetComponentsInChildren<TextMeshProUGUI> () [0].text =
                    GetRegionNameText (listRegion[i].Code);
                listRegionButton[i].GetComponentsInChildren<TextMeshProUGUI> () [1].text = "";
                listRegionButton[i].GetComponentsInChildren<Image> () [3].fillAmount = 0;
                int indexRegion = i;
                listRegionButton[indexRegion].onClick.AddListener (() =>
                {
                    OnRegionButtonClicked (listRegion[indexRegion].Code);
                });
                switch (listRegion[indexRegion].Code)
                {
                    case "asia":
                        listRegionButton[i].GetComponentsInChildren<Image> () [1].sprite = iconFlags[0];
                        break;
                    case "jp":
                        listRegionButton[i].GetComponentsInChildren<Image> () [1].sprite = iconFlags[1];
                        break;
                    case "kr":
                        listRegionButton[i].GetComponentsInChildren<Image> () [1].sprite = iconFlags[2];
                        break;
                    case "cn":
                        listRegionButton[i].GetComponentsInChildren<Image> () [1].sprite = iconFlags[3];
                        break;
                    case "rue":
                        listRegionButton[i].GetComponentsInChildren<Image> () [1].sprite = iconFlags[4];
                        break;
                }
            }
        }
    }
    private void OnRegionPingCompleted (RegionHandler regionHandler)
    {
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
        Debug.LogWarning ("OnJoinedLobby:" + PhotonNetwork.CurrentLobby.Name);
        cachedRoomList.Clear ();
        ClearRoomListView ();
    }
    // Called after leaving a lobby.
    public override void OnLeftLobby ()
    {
        Debug.LogWarning ("OnLeftLobby:" + PhotonNetwork.CurrentLobby.Name);
        cachedRoomList.Clear ();
        ClearRoomListView ();
    }
    // Called for any update of the room-listing while in a lobby (InLobby) on the Master Server.
    public override void OnRoomListUpdate (List<RoomInfo> roomList)
    {
        Debug.LogWarning ("OnRoomListUpdate: " + roomList.Count + " / " + PhotonNetwork.InLobby);
        textWarning.text = NAME_ROOM + "组织已更新成员列表。";
        ClearRoomListView ();
        UpdateCachedRoomList (roomList);
        UpdateRoomListView ();
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
        if (message == "A game with the specified id already exist.")
            textWarning.text = NAME_FAILED + NAME_CREATE + "：此行星已完成殖民，请直接前往。";
        else
            textWarning.text = NAME_FAILED + NAME_CREATE + message;

        ClearRoomListView ();
        btnRefreshRoomList.SetActive (true);
    }
    // Called when the LoadBalancingClient entered a room, no matter if this client created it or simply joined.
    public override void OnJoinedRoom ()
    {
        PhotonNetwork.SetMasterClient (PhotonNetwork.LocalPlayer);
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable { { PlayerCustomData.COMMAND, "Time/" + (DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second) } });
        // Debug.LogWarning ("OnJoinedRoom");
        textWarning.text = NAME_JOINED + PhotonNetwork.CurrentRoom.Name;
        textRoomName.text = PhotonNetwork.CurrentRoom.Name;
        textMaxPlayers.gameObject.SetActive (PhotonNetwork.CurrentRoom.MaxPlayers == 0 ? false : true);
        textMaxPlayers.text = PhotonNetwork.CurrentRoom.MaxPlayers.ToString ();
        roomIsOpen.SetActive (PhotonNetwork.CurrentRoom.IsOpen);
        roomIsVisible.SetActive (PhotonNetwork.CurrentRoom.IsVisible);
        panelLobby.SetActive (false);
        panelRoom.SetActive (true);

        foreach (Player player in PhotonNetwork.PlayerList)
        {
            if (player != PhotonNetwork.LocalPlayer && player.NickName != "iLYuSha")
            {
                GameObject entry = Instantiate (PlayerListEntryPrefab, PlayerListContent);
                entry.transform.localScale = Vector3.one;
                string nickName = player.NickName;
                entry.name = nickName;
                entry.GetComponentsInChildren<TextMeshProUGUI> () [0].text = nickName;
                entry.GetComponent<Button> ().onClick.AddListener (() =>
                {
                    FindObjectOfType<AgentCampManager> ().ChoosePlayer (nickName);
                });
                playerListEntries.Add (player.NickName, entry);
            }
        }
        if (numServices > 1)
        {
            if (PhotonNetwork.NickName == "Knights")
                timeCheckViolation = Time.time + Random.Range (60, 240);
        }
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

        ClearRoomListView ();
        btnRefreshRoomList.SetActive (true);
    }
    // Called when a previous OpJoinRandom call failed on the server.
    public override void OnJoinRandomFailed (short returnCode, string message)
    {
        // Debug.LogWarning ("OnJoinRandomFailed");
        if (message == "No match found")
            textWarning.text = NAME_FAILED + NAME_JOIN + "未知行星：找不到任何已殖民行星。";
        else
            textWarning.text = NAME_FAILED + NAME_JOIN + "未知行星" + message;
        // OnCreateRoomButtonClicked ();

        ClearRoomListView ();
        btnRefreshRoomList.SetActive (true);
    }
    // Called when the local user / client left a room, so the game 's logic can clean up it's internal state.
    public override void OnLeftRoom ()
    {
        Debug.LogWarning ("OnLeftRoom");
        textWarning.text = NAME_LEFT + textRoomName.text;
        panelLobby.SetActive (true);
        panelRoom.SetActive (false);

        foreach (GameObject entry in playerListEntries.Values)
        {
            Destroy (entry.gameObject);
        }
        playerListEntries.Clear ();
    }

    /// <summary>
    /// IInRoomCallbacks Interface 房間回傳介面
    /// </summary>
    // Called when a remote player entered the room.This Player is already added to the playerlist.
    public override void OnPlayerEnteredRoom (Player newPlayer)
    {
        if (newPlayer.NickName == "iLYuSha") return;
        // Debug.LogWarning ("OnPlayerEnteredRoom");
        textWarning.text = "殖民者 [" + newPlayer.NickName + "] " + NAME_JOINED + "本行星。";

        // 房間內的玩家按鈕
        GameObject entry = Instantiate (PlayerListEntryPrefab, PlayerListContent);
        entry.transform.localScale = Vector3.one;
        string nickName = newPlayer.NickName;
        entry.name = nickName;
        entry.GetComponentsInChildren<TextMeshProUGUI> () [0].text = nickName;
        entry.GetComponent<Button> ().onClick.AddListener (() =>
        {
            FindObjectOfType<AgentCampManager> ().ChoosePlayer (nickName);
        });
        playerListEntries.Add (newPlayer.NickName, entry);
        FindObjectOfType<AgentCampManager> ().ShowPlayerData ();

        if (PhotonNetwork.LocalPlayer == PhotonNetwork.MasterClient)
            PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable { { PlayerCustomData.COMMAND, "Time/" + (DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second) } });
    }
    // Called when a remote player left the room or became inactive.Check otherPlayer.IsInactive.
    public override void OnPlayerLeftRoom (Player otherPlayer)
    {
        if (otherPlayer.NickName == "iLYuSha") return;
        // Debug.LogWarning ("OnPlayerLeftRoom");
        textWarning.text = "殖民者 [" + otherPlayer.NickName + "] 已撤离本行星。";
        Destroy (playerListEntries[otherPlayer.NickName].gameObject);
        playerListEntries.Remove (otherPlayer.NickName);
        FindObjectOfType<AgentCampManager> ().ClearPlayer (otherPlayer);
    }
    // Called when a room 's custom properties changed. The propertiesThatChanged contains all that was set via Room.SetCustomProperties.
    public override void OnRoomPropertiesUpdate (Hashtable propertiesThatChanged)
    {
        Debug.LogWarning ("OnRoomPropertiesUpdate");
        textMaxPlayers.gameObject.SetActive (PhotonNetwork.CurrentRoom.MaxPlayers == 0 ? false : true);
        textMaxPlayers.text = PhotonNetwork.CurrentRoom.MaxPlayers.ToString ();
        roomIsOpen.SetActive (PhotonNetwork.CurrentRoom.IsOpen);
        roomIsVisible.SetActive (PhotonNetwork.CurrentRoom.IsVisible);
    }
    // Called when custom player - properties are changed.Player and the changed properties are passed as object[].
    public override void OnPlayerPropertiesUpdate (Player targetPlayer, Hashtable changedProps)
    {
        // Debug.LogWarning (targetPlayer.NickName + " Properties Update");
        // 以下控制代碼適用所有PC端觀測者，無需判定 MasterClient
        if (PhotonNetwork.InRoom)
            FindObjectOfType<AgentCampManager> ().DeveloperCommand ();
    }
    // Called after switching to a new MasterClient when the current one leaves.
    public override void OnMasterClientSwitched (Player newMasterClient)
    {
        if (newMasterClient.NickName == "iLYuSha") return;
        // Debug.LogWarning ("OnMasterClientSwitched");
        textWarning.text = "殖民者 [" + newMasterClient.NickName + "] 已成为本行星元首。";
        if (PhotonNetwork.LocalPlayer == newMasterClient)
            PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable { { PlayerCustomData.COMMAND, "Time/" + (DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second) } });
    }
    #endregion

    #region Advanced Method
    public static void SynchronizeClockTime ()
    {
        PhotonNetwork.MasterClient.SetCustomProperties (new Hashtable { { PlayerCustomData.MASTER_TIME, DateTime.Now.Hour * 3600 + DateTime.Now.Minute * 60 + DateTime.Now.Second } });
    }
    private void CloseRegion ()
    {
        for (int i = 0; i < listRegionButton.Count; i++)
        {
            listRegionButton[i].gameObject.SetActive (false);
        }
    }
    private void ClearRoomListView ()
    {
        foreach (GameObject entry in roomListEntries.Values)
        {
            Destroy (entry.gameObject);
        }

        roomListEntries.Clear ();
    }
    private void UpdateCachedRoomList (List<RoomInfo> roomList)
    {
        foreach (RoomInfo info in roomList)
        {
            // Remove room from cached room list if it got closed, became invisible or was marked as removed
            if (!info.IsOpen || !info.IsVisible || info.RemovedFromList)
            {
                if (cachedRoomList.ContainsKey (info.Name))
                {
                    cachedRoomList.Remove (info.Name);
                }

                continue;
            }

            // Update cached room info
            if (cachedRoomList.ContainsKey (info.Name))
            {
                cachedRoomList[info.Name] = info;
            }
            // Add new room info to cache
            else
            {
                cachedRoomList.Add (info.Name, info);
            }
        }
    }
    private void UpdateRoomListView ()
    {
        btnRefreshRoomList.SetActive (false);
        roomListContent.sizeDelta = new Vector2 (cachedRoomList.Count * 339 + 7, roomListContent.sizeDelta.y);
        foreach (RoomInfo info in cachedRoomList.Values)
        {
            GameObject entry = Instantiate (RoomListEntryPrefab, roomListContent.transform);
            entry.transform.localScale = Vector3.one;
            entry.GetComponent<Button> ().onClick.AddListener (() =>
            {
                if (PhotonNetwork.InLobby)
                {
                    PhotonNetwork.LeaveLobby ();
                }

                PhotonNetwork.JoinRoom (info.Name);
            });
            entry.GetComponentsInChildren<Image> () [0].sprite = iconPlanets[Random.Range (0, iconPlanets.Length)];
            entry.GetComponentsInChildren<TextMeshProUGUI> () [0].text = info.Name;
            entry.GetComponentsInChildren<TextMeshProUGUI> () [1].text = info.PlayerCount.ToString ();
            entry.GetComponentsInChildren<TextMeshProUGUI> () [2].text = "/ " + (info.MaxPlayers == 0 ? "#" : info.MaxPlayers.ToString ());
            roomListEntries.Add (info.Name, entry);
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
                return "狮城";
            case "jp":
                return "脚盆家";
            case "au":
                return "澳大利亚";
            case "sa":
                return "南美地区";
            case "in":
                return "印度";
            case "ru":
                return "俄罗斯";
            case "rue":
                return "伯力";
            case "kr":
                return "南棒";
            case "za":
                return "南非";
            case "cn":
                return "魔都";
            default:
                return "未知星系";
        }
    }
    private Player GetPlayerCustomPropetities (string code)
    {
        foreach (Player player in PhotonNetwork.PlayerList)
        {
            if (player.NickName == code)
                return player;
        }
        return null;
    }
    #endregion
}