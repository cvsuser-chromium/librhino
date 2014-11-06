package com.hybroad.dlna;

import android.text.TextUtils;

import com.hybroad.util.PublicConstants;

public class DLNAUtils {
    public static boolean isContainer(String classID) {
        if (TextUtils.isEmpty(classID)) {
            // By default, consider it as a directory when classID is null.
            return true;
        }

        if (classID.equals(DLNAConstant.Container.ALBUM))
            return true;
        if (classID.equals(DLNAConstant.Container.AUDIO_CHANNEL_GROUP))
            return true;
        if (classID.equals(DLNAConstant.Container.BOOKMARK_FOLDER))
            return true;
        if (classID.equals(DLNAConstant.Container.CHANNEL_GROUP))
            return true;
        if (classID.equals(DLNAConstant.Container.EPG_CONTAINER))
            return true;
        if (classID.equals(DLNAConstant.Container.GENRE))
            return true;
        if (classID.equals(DLNAConstant.Container.MOVIE_GENRE))
            return true;
        if (classID.equals(DLNAConstant.Container.MUSIC_ALBUM))
            return true;
        if (classID.equals(DLNAConstant.Container.MUSIC_ARTIST))
            return true;
        if (classID.equals(DLNAConstant.Container.MUSIC_GENRE))
            return true;
        if (classID.equals(DLNAConstant.Container.PERSON))
            return true;
        if (classID.equals(DLNAConstant.Container.PHOTO_ALBUM))
            return true;
        if (classID.equals(DLNAConstant.Container.PLAYLIST_CONTAINER))
            return true;
        if (classID.equals(DLNAConstant.Container.STORAGE_FOLDER))
            return true;
        if (classID.equals(DLNAConstant.Container.STORAGE_SYSTEM))
            return true;
        if (classID.equals(DLNAConstant.Container.STORAGE_VOLUME))
            return true;
        if (classID.equals(DLNAConstant.Container.VIDEO_CHANNEL_GROUP))
            return true;

        return false;
    }

    public static int convertToDMPSortID(int sortID) {
        switch (sortID) {
        case PublicConstants.SORT_BY_DATE:
            return DLNAConstant.Sort.BY_TIME;

        case PublicConstants.SORT_BY_SIZE:
            return DLNAConstant.Sort.BY_SIZE;

        case PublicConstants.SORT_BY_NAME:
        default:
            return DLNAConstant.Sort.BY_NAME;
        }
    }
}
