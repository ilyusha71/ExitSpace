using System.Collections.Generic;
using ExitGames.Client.Photon;
using Photon.Pun;
using Photon.Realtime;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class Spec : MonoBehaviourPunCallbacks
{

    public Sprite[] badge;
    public Image[] imgShow;
    public TextMeshProUGUI textAmount;
    public Text textNow;
    [Header ("Battle")]
    public bool isBattle;
    public GameObject solo, battle;
    public Image[] imgKGB, imgCIA;
    public TextMeshProUGUI textKGB, textCIA;

    Player[] players;
    public void Calculate ()
    {
        players = PhotonNetwork.PlayerList;
        int total = 0;
        for (int i = 0; i < players.Length; i++)
        {
            object customData;
            for (int j = 0; j < 10; j++)
            {
                if (players[i].CustomProperties.TryGetValue (PlayerCustomData.BADGE[j], out customData))
                    if ((bool) customData)
                    {
                        imgShow[total].sprite = badge[j];
                        imgShow[total].enabled = true;
                        total++;
                    }
            }
        }
        for (int i = total; i < imgShow.Length; i++)
        {
            imgShow[i].enabled = false;
        }
        textAmount.text = total.ToString ();

        if (total >= (players.Length - 2) * 10)
            textNow.text = "恭喜過關，完美表現";
        else if (total >= (players.Length - 2) * 8)
            textNow.text = "恭喜過關";
        else if (total >= (players.Length - 2) * 6)
            textNow.text = "加油！就差臨門一腳";
        else if (total >= (players.Length - 2) * 4)
            textNow.text = "再接再厲";
        else if (total >= (players.Length - 2) * 2)
            textNow.text = "好的開始，繼續努力";
        else if (total >= (players.Length - 2) * 0)
            textNow.text = "考驗開始，衝鴨";
    }

    public void Battle ()
    {
        players = PhotonNetwork.PlayerList;
        int totalKGB = 0;
        int totalCIA = 0;
        for (int i = 0; i < players.Length; i++)
        {
            object customData;
            for (int j = 0; j < 10; j++)
            {
                if (players[i].CustomProperties.TryGetValue (PlayerCustomData.BADGE[j], out customData))
                    if ((bool) customData)
                    {
                        if (players[i].NickName.Contains ("KGB"))
                        {
                            imgKGB[totalKGB].sprite = badge[j];
                            imgKGB[totalKGB].enabled = true;
                            totalKGB++;
                        }
                        else if (players[i].NickName.Contains ("CIA"))
                        {
                            imgCIA[totalCIA].sprite = badge[j];
                            imgCIA[totalCIA].enabled = true;
                            totalCIA++;
                        }
                    }
            }
        }
        for (int i = totalKGB; i < imgKGB.Length; i++)
        {
            imgKGB[i].enabled = false;
        }
        textKGB.text = totalKGB.ToString ();
        for (int i = totalCIA; i < imgCIA.Length; i++)
        {
            imgCIA[i].enabled = false;
        }
        textCIA.text = totalCIA.ToString ();
    }
    public override void OnPlayerPropertiesUpdate (Player targetPlayer, Hashtable changedProps)
    {
        // Debug.LogWarning ("OnPlayerPropertiesUpdate");
        if (isBattle)
            Battle ();
        else
            Calculate ();
    }

    void Update ()
    {
        if (Input.GetKeyDown (KeyCode.F9))
        {
            solo.SetActive (!solo.activeSelf);
            battle.SetActive (!battle.activeSelf);
            isBattle = !isBattle;
        }
    }
}