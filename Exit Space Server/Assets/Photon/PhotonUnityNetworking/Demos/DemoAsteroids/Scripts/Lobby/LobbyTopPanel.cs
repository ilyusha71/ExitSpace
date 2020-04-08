using UnityEngine;
using UnityEngine.UI;

namespace Photon.Pun.Demo.Asteroids
{
    public class LobbyTopPanel : MonoBehaviour
    {
        private readonly string connectionStatusMessage = "    Connection Status: ";

        [Header ("UI References")]
        public Text ConnectionStatusText;
        public Text tt;
        public Text lobby;

        public Text tt0;
        public Text tt1;
        public Text tt2;
        public Text tt3;
        public Text tt4;

        #region UNITY

        public void Update ()
        {
            ConnectionStatusText.text = connectionStatusMessage + PhotonNetwork.NetworkClientState;
            tt.text = PhotonNetwork.NetworkingClient.CloudRegion + " / " + PhotonNetwork.GetPing ();
            if (PhotonNetwork.InLobby)
            {
                Debug.Log("sdsdsd");
                lobby.text = PhotonNetwork.CurrentLobby.Name;

            }
            tt0.text = "ServerAddress\n" + PhotonNetwork.ServerAddress;
            tt1.text = "CurrentServerAddress\n" + PhotonNetwork.NetworkingClient.CurrentServerAddress;
            tt2.text = "GameServerAddress\n" + PhotonNetwork.NetworkingClient.GameServerAddress;
            tt3.text = "MasterServerAddress\n" + PhotonNetwork.NetworkingClient.MasterServerAddress;
            tt4.text = "NameServerAddress\n" + PhotonNetwork.NetworkingClient.NameServerAddress;

        }

        #endregion
    }
}