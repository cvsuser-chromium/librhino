package com.hybroad.dlna;

public class DLNAConstant {

    public static final class Sort {
        public static final int BY_NAME = 0;
        public static final int BY_SIZE = 1;
        public static final int BY_TYPE = 2;
        public static final int BY_TIME = 3;
    }

    public static final class Order {
        public static final int ASCENDING = 0;
        public static final int DESCENDING = 1;
    }

    public static final class Container {
        public static final String PERSON = "a1";
        public static final String MUSIC_ARTIST = "a11";
        public static final String PLAYLIST_CONTAINER = "a2";
        public static final String ALBUM = "a3";
        public static final String MUSIC_ALBUM = "a31";
        public static final String PHOTO_ALBUM = "a32";
        public static final String GENRE = "a4";
        public static final String MUSIC_GENRE = "a41";
        public static final String MOVIE_GENRE = "a42";
        public static final String CHANNEL_GROUP = "a5";
        public static final String AUDIO_CHANNEL_GROUP = "a51";
        public static final String VIDEO_CHANNEL_GROUP = "a52";
        public static final String EPG_CONTAINER = "a6";
        public static final String STORAGE_SYSTEM = "a7";
        public static final String STORAGE_VOLUME = "a8";
        public static final String STORAGE_FOLDER = "a9";
        public static final String BOOKMARK_FOLDER = "aa";
    }

    public static final class Item {
        public static final String IMAGE_ITEM = "1";
        public static final String PHOTO = "11";
        public static final String AUDIO_ITEM = "2";
        public static final String MUSIC_TRACK = "21";
        public static final String AUDIO_BROADCAST = "22";
        public static final String AUDIO_BOOK = "23";
        public static final String AUDIO_DOWNLOAD = "24";
        public static final String VIDEO_ITEM = "3";
        public static final String MOVIE = "31";
        public static final String VIDEO_BROADCAST = "32";
        public static final String MUSIC_VIDEO_CLIP = "33";
        public static final String CPVR = "34";
        public static final String VIDEO_DOWNLOADS = "35";
        public static final String BOOKMARK_ITEM = "4";
        public static final String PLAYLIST_ITEM = "6";
        public static final String TEXT_ITEM = "7";
        public static final String EPG_IEM = "8";
        public static final String AUDIO_PROGRAM = "81";
        public static final String VIDEO_PRORAM = "82";
    }

    public static final class Channel {
        public static final String STEREO = "Master";
        public static final String LEFT_FRONT = "LF";
        public static final String RIGHT_FRONT = "RF";
    }

}
