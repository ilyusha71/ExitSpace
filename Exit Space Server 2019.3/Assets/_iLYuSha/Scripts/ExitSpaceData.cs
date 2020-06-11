public class ExitSpaceData
{
    public static readonly string[] STAGE_1_ENTRY = new string[]
    {
        "1-N1-X",
        "1-N2-X",
    };
    public static readonly string[] STAGE_2_ENTRY = new string[]
    {
        "2-C2-W",
        "2-C4-W",
        "2-C6-X",
    };
    public static readonly string[] STAGE_3_ENTRY = new string[]
    {
        "3-E6-E4",
        "3-U1-X",
    };
    public static readonly string[] STAGE_4_ENTRY = new string[]
    {
        "4-W-C1",
    };
    public static readonly string[] MERLIN_CABINET = new string[]
    {
        "1-U1-X",
    };
    public static readonly string[] DOORS = new string[]
    {
        "2-U3-U2",
        "2-U7-U5",
        "2-U7-U3",
    };
    public static readonly string[] TRAP_DOORS = new string[]
    {
        "H1-W-C1", // Room 15
        "H1-W-U8", // Room 15
        "V1-A910-W", // Room 15
        "V1-W-C1", // Room 16
        "H3-W-U7", // Room 16
        "V3-W-U7", // Room 16
        "V3-W-C7", // Room 17
        "V3-W-C7U6", // Room 17
        "H4-W-C3", // Room 17
        "V4-C8-X", // Room 17
        "H5-X-C2", // Room 17
        "V6-W-A189", // Room 18
        "H8-W-U8", // Room 18
        "V8-W-U6", // Room 18   
        "H8-W-C1", // Room 19
        "V8-W-W", // Room 19
    };
    public static readonly string[] MAZE_DOORS = new string[]
    {
        "V1-A89-C1",
        "H2-U10-U3",
        "H2-R610-U5",
        "V2-U4-U5",
        "V2-U3-A67",
        "V2-C3-A345",
        "H3-U4-A57",
        "H3-A78-C2",
        "V3-R457-C2",
        "V3-C4-U8",
        "H4-U9-A45",
        "H4-C2-U3",
        "V4-C2-C1",
        "V4-U10-C3",
        "H5-C2-C1",
        "V5-U6-C4",
        "V5-U3-C8",
        "H6-U7-U6",
        "V6-U5-U7",
        "V7-U4-C3",
        "V8-U3-C4",
        "H9-U3-C1",
    };
    public static readonly string[] TRAP_18_DOORS = new string[]
    {
        "V6-W-A189", // Room 18
        "H8-W-U8", // Room 18
        "V8-W-U6", // Room 18
    };
    public static readonly string[] CHALLENGE_BOX = new string[]
    {
        "4B1-A189-T1", // Arthur
        "4B2-A189-T1", // Arthur
        "4B1-A1210-T2", // Merlin
        "4B2-A1210-T2", // Merlin
        "4B1-A347-T3", // Lancelot
        "4B2-A347-T3", // Lancelot
        "4B3-A347-T3", // Lancelot
        "4B1-A456-T4", // Galahad
        "4B2-A456-T4", // Galahad
    };
    public static readonly string[] PRINTER = new string[]
    {
        "1-P0-X",
        "3-P1-X",
        "3-P2-X",
        "3-P3-X",
    };
    public static readonly string[] WRITER = new string[]
    {
        "1-B1-X",
        "1-B2-X",
        "1-B3-X",
        "1-B4-X",
        "1-B5-X",
        "1-B6-X",
        "1-B7-X",
        "2-B8-X",
        "2-B9-X",
        "2-B10-X",
    };
    public static readonly string[] CALLBACK_WRITER = new string[]
    {
        "900", // 1-B1-X
        "1000", // 1-B2-X
        "300", // 1-B3-X
        "400", // 1-B4-X
        "500", // 1-B5-X
        "600", // 1-B6-X
        "700", // 1-B7-X
        "550", // 2-B8-X
        "350", // 2-B9-X
        "200", // 2-B10-X
    };
    public static readonly string[] CALLBACK_BOX = new string[]
    {
        "130", //"4B1-A189-T1", // Arthur
        "140", //"4B2-A189-T1", // Arthur
        "230", //"4B1-A1210-T2", // Merlin
        "240", //"4B2-A1210-T2", // Merlin
        "330", //"4B1-A347-T3", // Lancelot
        "340", //"4B2-A347-T3", // Lancelot
        "350", //"4B3-A347-T3", // Lancelot
        "430", //"4B1-A456-T4", // Galahad
        "440", //"4B2-A456-T4", // Galahad
    };

    public static string GetTitle (string t)
    {
        switch (t)
        {
            case "T1":
                return "Arthur";
            case "T2":
                return "Merlin";
            case "T3":
                return "Lancelot";
            case "T4":
                return "Galahad";
        }
        return "";
    }

    public static string GetWriterCallback (string device)
    {
        for (int i = 0; i < WRITER.Length; i++)
        {
            if (device == WRITER[i])
                return CALLBACK_WRITER[i];
        }
        return "2000";
    }

    public static string GetBoxCallback (string device)
    {
        for (int i = 0; i < CHALLENGE_BOX.Length; i++)
        {
            if (device == CALLBACK_BOX[i])
                return CALLBACK_BOX[i];
        }
        return "2000";
    }

    public static bool IsWriter (string device)
    {
        for (int i = 0; i < WRITER.Length; i++)
        {
            if (device == WRITER[i])
                return true;
        }
        return false;
    }

    public static bool IsStage1Entry (string device)
    {
        for (int i = 0; i < STAGE_1_ENTRY.Length; i++)
        {
            if (device == STAGE_1_ENTRY[i])
                return true;
        }
        return false;
    }

    public static bool IsStage2Entry (string device)
    {
        for (int i = 0; i < STAGE_2_ENTRY.Length; i++)
        {
            if (device == STAGE_2_ENTRY[i])
                return true;
        }
        return false;
    }

    public static bool IsStage3Entry (string device)
    {
        for (int i = 0; i < STAGE_3_ENTRY.Length; i++)
        {
            if (device == STAGE_3_ENTRY[i])
                return true;
        }
        return false;
    }

    public static bool IsStage4Entry (string device)
    {
        for (int i = 0; i < STAGE_4_ENTRY.Length; i++)
        {
            if (device == STAGE_4_ENTRY[i])
                return true;
        }
        return false;
    }

    public static bool IsMerlinCabinet (string device)
    {
        for (int i = 0; i < MERLIN_CABINET.Length; i++)
        {
            if (device == MERLIN_CABINET[i])
                return true;
        }
        return false;
    }

    public static bool IsDoor (string device)
    {
        for (int i = 0; i < DOORS.Length; i++)
        {
            if (device == DOORS[i])
                return true;
        }
        return false;
    }

    public static bool IsUnlockTrapDoor (string device)
    {
        for (int i = 0; i < TRAP_DOORS.Length; i++)
        {
            if (device == TRAP_DOORS[i])
                return true;
        }
        return false;
    }

    public static bool IsUnlockMazeDoor (string device)
    {
        for (int i = 0; i < MAZE_DOORS.Length; i++)
        {
            if (device == MAZE_DOORS[i])
                return true;
        }
        return false;
    }

    public static bool IsTrap18Door (string device)
    {
        for (int i = 0; i < TRAP_18_DOORS.Length; i++)
        {
            if (device == TRAP_18_DOORS[i])
                return true;
        }
        return false;
    }

    public static bool IsChallengeBox (string device)
    {
        for (int i = 0; i < CHALLENGE_BOX.Length; i++)
        {
            if (device == CHALLENGE_BOX[i])
                return true;
        }
        return false;
    }
}