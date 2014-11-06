package com.hybroad.image;

import java.util.Comparator;

import com.hybroad.util.PinyinUtil;

import android.os.Parcel;
import android.os.Parcelable;

public class ImageFile implements Parcelable {
    private int type; // 0:image 1:directory

    public int imageId;
    public String filePath;
    public String thumbPath;
    public String title;
    public String dateTaken;
    public long dateTime;
    public long size;

    // for DMSFile
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
        dest.writeInt(imageId);
        dest.writeString(filePath);
        dest.writeString(thumbPath);
        dest.writeString(title);
        dest.writeString(dateTaken);
        dest.writeLong(dateTime);
        dest.writeLong(size);

        dest.writeString(objectID);
        dest.writeString(classID);
        dest.writeString(parentID);
    }

    public static final Parcelable.Creator<ImageFile> CREATOR = new Parcelable.Creator<ImageFile>() {
        public ImageFile createFromParcel(Parcel in) {
            ImageFile imageFile = new ImageFile();

            imageFile.type = in.readInt();
            imageFile.imageId = in.readInt();
            imageFile.filePath = in.readString();
            imageFile.thumbPath = in.readString();
            imageFile.title = in.readString();
            imageFile.dateTaken = in.readString();
            imageFile.dateTime = in.readLong();
            imageFile.size = in.readLong();

            imageFile.objectID = in.readString();
            imageFile.classID = in.readString();
            imageFile.parentID = in.readString();

            return imageFile;
        }

        public ImageFile[] newArray(int size) {
            return new ImageFile[size];
        }
    };

    public boolean isImage() {
        return (0 == type) ? true : false;
    }

    public boolean isFolder() {
        return (1 == type) ? true : false;
    }

    public void setTypeImage() {
        this.type = 0;
    }

    public void setTypeFolder() {
        this.type = 1;
    }

    private Comparator<ImageFile> mDateTimeComparator = new Comparator<ImageFile>() {
        public int compare(ImageFile arg0, ImageFile arg1) {
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

    public Comparator<ImageFile> getDateTimeComparator() {
        return this.mDateTimeComparator;
    }

    private Comparator<ImageFile> mTitleComparator = new Comparator<ImageFile>() {
        public int compare(ImageFile arg0, ImageFile arg1) {
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

    public Comparator<ImageFile> getTitleComparator() {
        return this.mTitleComparator;
    }

    private Comparator<ImageFile> mSizeComparator = new Comparator<ImageFile>() {
        public int compare(ImageFile arg0, ImageFile arg1) {
            int result = 0;
            if (arg0.size > arg1.size) {
                result = 1;
            } else if (arg0.size < arg1.size) {
                result = -1;
            }
            return result;
        }
    };

    public Comparator<ImageFile> getSizeComparator() {
        return this.mSizeComparator;
    }
}
