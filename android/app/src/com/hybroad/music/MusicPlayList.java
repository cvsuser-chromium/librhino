package com.hybroad.music;

import java.util.ArrayList;

public class MusicPlayList {
    public static ArrayList<MusicFile> sCurrentMusicList = null;
    public static ArrayList<MusicFile> sCurrentMusicListBackup = null;

    public static synchronized ArrayList<MusicFile> getListData() {
        return MusicPlayList.sCurrentMusicList;
    }

    public static synchronized void setListData(ArrayList<MusicFile> list) {
        MusicPlayList.sCurrentMusicList = list;
    }
}
