using System.Collections;
using System.Collections.Generic;
using ExitGames.Client.Photon;
using Photon.Pun;
using Photon.Realtime;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class Lobby : MonoBehaviourPunCallbacks
{
    string iniAPP, iniNet, FFGame, FFNet;

    public TextMeshProUGUI address;

    public TextMeshProUGUI lobbyName;

    #region Login
    public TextMeshProUGUI textPUNCloud, textServer, textRegionPing;
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
    private int roomIndex = 1;

    #endregion

    public void ConnectTest ()
    {
        PhotonNetwork.LocalPlayer.NickName = "Test01";
        PhotonNetwork.ConnectUsingSettings ();
    }

    void Awake ()
    {
        Button[] btns = groupRegion.GetComponentsInChildren<Button> ();
        for (int i = 0; i < btns.Length; i++)
        {
            listRegionButton.Add (btns[i]);
            btns[i].gameObject.SetActive (false);
        }
        inputCode.text = PlayerPrefs.GetString ("InputCode");
    }

    void New ()
    {
        // PhotonNetwork.NetworkingClien
    }

    void Update ()
    {
        textPUNCloud.text = PhotonNetwork.NetworkClientState.ToString ();
        if (PhotonNetwork.IsConnected)
        {
            textServer.text = PhotonNetwork.CloudRegion;
            textRegionPing.text = PhotonNetwork.GetPing () + " ms";
            address.text = PhotonNetwork.ServerAddress;
            if (PhotonNetwork.InLobby)
            {
                Debug.Log ("sdsdsd");
                lobbyName.text = PhotonNetwork.CurrentLobby.Name;
            }

            // Debug.Log (PhotonNetwork.NetworkingClient.EnableLobbyStatistics);

        }
        else
        {
            textServer.text = PhotonNetwork.CloudRegion;
            textRegionPing.text = "";
        }
        if (showRegion)
        {
            showRegion = false;
            tipSearchRegionServer.SetActive (false);
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
                    GetRegionName (listRegion[i].Code);
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
    public void OnInternationServerButtonClicked ()
    {
        tipSearchRegionServer.SetActive (true);
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
            // PhotonNetwork.ConnectToRegion (region);
        }
        else
        {
            Debug.LogError ("input Code is invalid.");
        }
    }

    public void OnCreateRoomButtonClicked ()
    {
        // string roomName = "123" + roomIndex;
        string roomName = "123";

        roomName = (roomName.Equals (string.Empty)) ? "Room " + Random.Range (1000, 10000) : roomName;

        byte maxPlayers;
        byte.TryParse ("10", out maxPlayers);
        maxPlayers = (byte) Mathf.Clamp (maxPlayers, 2, 8);

        RoomOptions options = new RoomOptions { MaxPlayers = maxPlayers };

        PhotonNetwork.CreateRoom (roomName, options, null);
    }

    public void OnJoinRoomButtonClicked ()
    {
        // string roomName = "123" + roomIndex;
        string roomName = "123";
        PhotonNetwork.JoinRoom (roomName);
    }

    public void OnJoinCRoomButtonClicked ()
    {
        // string roomName = "123" + roomIndex;
        string roomName = "123";
        PhotonNetwork.JoinOrCreateRoom (roomName, null, null, null);
    }

    public void OnLeftRoomButtonClicked ()
    {
        PhotonNetwork.LeaveRoom ();
    }
    #endregion

    #region PUN CALLBACKS
    public override void OnRegionListReceived (RegionHandler regionHandler)
    {
        regionHandler.PingMinimumOfRegions (this.OnRegionPingCompleted, null);
    }

    private void OnRegionPingCompleted (RegionHandler regionHandler)
    {
        listRegion = new List<Region> (regionHandler.EnabledRegions);
        Debug.Log ("OnRegionPingCompleted " + regionHandler.BestRegion);
        Debug.Log ("RegionPingSummary: " + regionHandler.SummaryToCache);
        showRegion = true;
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
        Debug.LogWarning ("OnRoomListUpdate");
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
        Debug.LogWarning ("OnCreatedRoom");
    }
    // Called when the server couldn't create a room (OpCreateRoom failed).
    public override void OnCreateRoomFailed (short returnCode, string message)
    {
        Debug.LogWarning ("OnCreateRoomFailed");
    }
    // Called when the LoadBalancingClient entered a room, no matter if this client created it or simply joined.
    public override void OnJoinedRoom ()
    {
        Debug.LogWarning ("OnJoinedRoom");
    }
    // Called when a previous OpJoinRoom call failed on the server.
    public override void OnJoinRoomFailed (short returnCode, string message)
    {
        Debug.LogWarning ("OnJoinRoomFailed");
    }
    // Called when a previous OpJoinRandom call failed on the server.
    public override void OnJoinRandomFailed (short returnCode, string message)
    {
        Debug.LogWarning ("OnJoinRandomFailed");
    }
    // Called when the local user / client left a room, so the game 's logic can clean up it's internal state.
    public override void OnLeftRoom ()
    {
        Debug.LogWarning ("OnLeftRoom");
    }

    #endregion

    void CloseRegion ()
    {
        for (int i = 0; i < listRegionButton.Count; i++)
        {
            listRegionButton[i].gameObject.SetActive (false);
        }
    }

    private string GetRegionName (string region)
    {
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
                return "未知区域";
        }
    }
}