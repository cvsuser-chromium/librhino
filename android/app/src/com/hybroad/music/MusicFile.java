package com.hybroad.music;

import java.util.Comparator;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

import com.hybroad.util.PinyinUtil;

public class MusicFile implements Parcelable {
    private int type; // 0:music 1:directory

    public int listPosition;
    public int duration;
    public int album_id = -1;
    public int song_id = -1;
    public String filePath;
    public String thumbPath;
    public String title;
    public String artist = null; // donot init empty string ""
    public int embededImageExists = 1; // 1:exists 0:not
    public long dateTime;
    public long size;

    public String objectID;
    public String classID;
    public String parentID;

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(type);
        dest.writeInt(listPosition);
        dest.writeInt(duration);
        dest.writeInt(album_id);
        dest.writeInt(song_id);
        dest.writeString(filePath);
        dest.writeString(thumbPath);
        dest.writeString(title);
        dest.writeString(artist);
        dest.writeInt(embededImageExists);
        dest.writeLong(dateTime);
        dest.writeLong(size);

        dest.writeString(objectID);
        dest.writeString(classID);
        dest.writeString(parentID);
    }

    public static final Parcelable.Creator<MusicFile> CREATOR = new Parcelable.Creator<MusicFile>() {
        public MusicFile createFromParcel(Parcel in) {
            MusicFile musicFile = new MusicFile();

            musicFile.type = in.readInt();
            musicFile.listPosition = in.readInt();
            musicFile.duration = in.readInt();
            musicFile.album_id = in.readInt();
            musicFile.song_id = in.readInt();
            musicFile.filePath = in.readString();
            musicFile.thumbPath = in.readString();
            musicFile.title = in.readString();
            musicFile.artist = in.readString();
            musicFile.embededImageExists = in.readInt();
            musicFile.dateTime = in.readLong();
            musicFile.size = in.readLong();

            musicFile.objectID = in.readString();
            musicFile.classID = in.readString();
            musicFile.parentID = in.readString();

            return musicFile;
        }

        public MusicFile[] newArray(int size) {
            return new MusicFile[size];
        }
    };

    public boolean isMusic() {
        return (0 == type) ? true : false;
    }

    public boolean isFolder() {
        return (1 == type) ? true : false;
    }

    public void setTypeMusic() {
        this.type = 0;
    }

    public void setTypeFolder() {
        this.type = 1;
    }

    private Comparator<MusicFile> mDateTimeComparator = new Comparator<MusicFile>() {
        public int compare(MusicFile arg0, MusicFile arg1) {
            int result = 0;
            if (arg0 == null || arg1 == null) {
                return result;
            }
            if (arg0.dateTime > arg1.dateTime) {
                result = 1;
            } else if (arg0.dateTime < arg1.dateTime) {
                result = -1;
            }
            return result;
        }
    };

    public Comparator<MusicFile> getDateTimeComparator() {
        return this.mDateTimeComparator;
    }

    private Comparator<MusicFile> mTitleComparator = new Comparator<MusicFile>() {
        public int compare(MusicFile arg0, MusicFile arg1) {
            if (arg0 == null || arg1 == null) {
                return 0;
            }
            String s1 = PinyinUtil.hanZiToPinYin(arg0.title.toLowerCase());
            String s2 = PinyinUtil.hanZiToPinYin(arg1.title.toLowerCase());
            return s1.compareTo(s2);
            // Log.d("--compare--", s1 + " <--> " + s2);
            // return s2.compareTo(s1);
        }
    };

    public Comparator<MusicFile> getTitleComparator() {
        return this.mTitleComparator;
    }

    private Comparator<MusicFile> mSizeComparator = new Comparator<MusicFile>() {
        public int compare(MusicFile arg0, MusicFile arg1) {
            int result = 0;
            if (arg0.size > arg1.size) {
                result = 1;
            } else if (arg0.size < arg1.size) {
                result = -1;
            }
            return result;
        }
    };

    public Comparator<MusicFile> getSizeComparator() {
        return this.mSizeComparator;
    }
}
