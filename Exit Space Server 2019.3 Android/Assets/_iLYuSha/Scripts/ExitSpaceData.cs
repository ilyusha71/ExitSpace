public class ExitSpaceData
{
    public static readonly string[] STAGE_1_Entry = new string[]
    {
        "WakakaEntry1",
        "WakakaEntry2",
    };
    public static readonly string[] STAGE_2_Entry = new string[]
    {
        "2-C2-W",
        "2-C4-W",
        "2-C6-X",
    };
    public static readonly string[] STAGE_3_Entry = new string[]
    {
        "3-E6-E4",
        "3-U1-X",
    };
    public static readonly string[] STAGE_4_Entry = new string[]
    {
        "4-W-C1",
    };
    public static readonly string[] DOORS = new string[]
    {
        "1-U1-X",
        "2-U3-U2",
        "2-U7-U5",
        "2-U7-U3",
        "1-U1-X",
        "1-U1-X",
        "1-U1-X",
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
    public static readonly string[] TRAP_DOORS = new string[]
    {
        "H1-W-C1", // Room 15
        "H1-W-U8", // Room 15
        "V1-A910-W", // Room 15
        "V1-W-C1", // Room 16
        "H3-W-U7", // Room 16
        "V3-W-U7", // Room 16
        "V3-W-C7U6", // Room 17
        "H4-W-C3", // Room 17
        "H5-X-C2", // Room 17
        "V6-W-A189", // Room 18
        "H8-W-U8", // Room 18
        "V8-W-U6", // Room 18
    };
    public static readonly string[] CHALLENGE_BOX = new string[]
    {
        "4-A1210-T0", // Merlin
        "4-A1210-T1", // Merlin
        "4-A189-T0", // Arthur
        "4-A189-T1", // Arthur
        "4-A347-T0", // Lancelot
        "4-A347-T1", // Lancelot
        "4-A347-T2", // Lancelot
        "4-A456-T0", // Galahad
        "4-A456-T1", // Galahad
    };

    public static bool IsStage1Entry (string device)
    {
        for (int i = 0; i < STAGE_1_Entry.Length; i++)
        {
            if (device == STAGE_1_Entry[i])
                return true;
        }
        return false;
    }

    public static bool IsStage2Entry (string device)
    {
        for (int i = 0; i < STAGE_2_Entry.Length; i++)
        {
            if (device == STAGE_2_Entry[i])
                return true;
        }
        return false;
    }

    public static bool IsStage3Entry (string device)
    {
        for (int i = 0; i < STAGE_3_Entry.Length; i++)
        {
            if (device == STAGE_3_Entry[i])
                return true;
        }
        return false;
    }

    public static bool IsStage4Entry (string device)
    {
        for (int i = 0; i < STAGE_4_Entry.Length; i++)
        {
            if (device == STAGE_4_Entry[i])
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

    public static bool IsUnlockMazeDoor (string device)
    {
        for (int i = 0; i < MAZE_DOORS.Length; i++)
        {
            if (device == MAZE_DOORS[i])
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

    public static bool IsChallengeBox (string device)
    {
        for (int i = 0; i < CHALLENGE_BOX.Length; i++)
        {
            if (device == CHALLENGE_BOX[i])
                return true;
        }
        return false;
    }

    public static readonly string[] TR000AP = new string[]
    {
        "WAKAKA",
        "1-U1-X",
        "2-C2-W",
        "2-C4-W",
        "2-C6-X",
        "2-U3-U2",
        "2-U7-U3",
        "2-U7-U5",
        "2-U1-X",
        "2-E6-E4",
        "H1-W-C1",
        "H1-W-U8",
        "V1-A89-C1",
        "V1-A910-W",
        "V1-W-C1",
        "H2-U10-U3",
        "H2-R610-U5",
        "V2-U4-U5",
        "V2-U3-A67",
        "V2-C3-A345",
        "H3-U4-A57",
        "H3-A78-C2",
        "H3-W-U7",
        "V3-W-C7U6",
        "V3-R457-C2",
        "V3-C4-U8",
        "H4-W-C3",
        "H4-U9-A45",
        "H4-C2-U3",
        "V4-C8-X",
        "V4-C2-C1",
        "V4-U10-C3",
        "H5-X-C2",
        "H5-C2-C1",
        "V5-U6-C4",
        "V5-U3-C8",
        "H6-U7-U6",
        "V6-W-A189",
        "V6-U5-U7",
        "V7-U4-C3",
        "H8-W-C1",
        "H8-W-U8",
        "V8-W-W",
        "V8-W-U6",
        "V8-U3-C4",
        "H9-U3-C1",
        "4-W-C1",
        "4-A456-T0",
        "4-A1210-T0",
        "4-A347-T0",
        "4-A456-T1",
        "4-A347-T1",
        "4-A189-T0",
        "4-A189-T1",
        "4-A1210-T1",
        "4-A347-T2",
    };

    public static readonly string[] LOCK_READERS = new string[]
    {
        "WAKAKA",
        "1-U1-X",
        "2-C2-W",
        "2-C4-W",
        "2-C6-X",
        "2-U3-U2",
        "2-U7-U3",
        "2-U7-U5",
        "2-U1-X",
        "2-E6-E4",
        "H1-W-C1",
        "H1-W-U8",
        "V1-A89-C1",
        "V1-A910-W",
        "V1-W-C1",
        "H2-U10-U3",
        "H2-R610-U5",
        "V2-U4-U5",
        "V2-U3-A67",
        "V2-C3-A345",
        "H3-U4-A57",
        "H3-A78-C2",
        "H3-W-U7",
        "V3-W-C7U6",
        "V3-R457-C2",
        "V3-C4-U8",
        "H4-W-C3",
        "H4-U9-A45",
        "H4-C2-U3",
        "V4-C8-X",
        "V4-C2-C1",
        "H5-X-C2",
        "H5-C2-C1",
        "V5-U6-C4",
        "V5-U3-C8",
        "H6-U7-U6",
        "V6-W-A189",
        "V6-U5-U7",
        "V7-U4-C3",
        "H8-W-C1",
        "H8-W-U8",
        "V8-W-W",
        "V8-W-U6",
        "V8-U3-C4",
        "H9-U3-C1",
        "4-W-C1",
        "4-A456-T0",
        "4-A1210-T0",
        "4-A347-T0",
        "4-A456-T1",
        "4-A347-T1",
        "4-A189-T0",
        "4-A189-T1",
        "4-A1210-T1",
        "4-A347-T2",
    };

    public static readonly string[] WRITER = new string[]
    {
        "WAKAKA",
        "ARTHUR",
        "MERLIN",
        "LANCELOT",
        "GALAHAD",
        "PERCIVAL",
        "BORS",
        "GUINEVERE",
        "EXCALIBUR",
        "SWORD",
        "VIVIANE",
        "4-A456-T0",
        "4-A1210-T0",
        "4-A347-T0",
        "4-A456-T1",
        "4-A347-T1",
        "4-A189-T0",
        "4-A189-T1",
        "4-A1210-T1",
        "4-A347-T2",
    };
}