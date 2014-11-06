package com.hybroad.video;

import java.util.ArrayList;
import java.util.Random;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public class VideoPlayList implements Parcelable {
  private String TAG = "VideoPlayList";
  private ArrayList<VideoFile> mVideoList = null;
  private String mCurrentPath = null;
  private int mCurrentPosition = 0;
  private boolean isListChanged = false;

  public VideoPlayList(String currentPath, int currentPosition) {
    this.mCurrentPath = currentPath;
    this.mCurrentPosition = currentPosition;
  }

  public void setList() {
    this.mVideoList = (ArrayList<VideoFile>) CommonData.getVideoList();
  }

  public VideoFile getCurrentVideo() {
    if (mVideoList == null || mCurrentPosition < 0
        || mCurrentPosition >= mVideoList.size()) {
      return null;
    }
    return mVideoList.get(mCurrentPosition);
  }

  public VideoFile getNextVideo() {
    VideoFile video = null;
    checkVideoList();
    if (isListChanged) {
      for (int i = 0; i < mVideoList.size(); i++) {
        if (mVideoList.get(i).path.equals(mCurrentPath)) {
          if (i + 1 >= mVideoList.size()) {
            mCurrentPosition = 0;
            mCurrentPath = mVideoList.get(mCurrentPosition).path;
            video = mVideoList.get(0);
          } else {
            mCurrentPosition = i + 1;
            mCurrentPath = mVideoList.get(mCurrentPosition).path;
            video = mVideoList.get(mCurrentPosition);
          }
          break;
        }
      }
      isListChanged = false;
    } else {
      mCurrentPosition += 1;
      if (mCurrentPosition >= mVideoList.size()) {
        mCurrentPosition = 0;
      }
      mCurrentPath = mVideoList.get(mCurrentPosition).path;
      video = mVideoList.get(mCurrentPosition);
    }

    return video;
  }

  public VideoFile getNextVideo_NoCycle() {

    VideoFile video = null;
    checkVideoList();
    if (isListChanged) {
      for (int i = 0; i < mVideoList.size(); i++) {
        if (mVideoList.get(i).path.equals(mCurrentPath)) {
          if (i + 1 >= mVideoList.size()) {
            mCurrentPosition = 0;
            mCurrentPath = mVideoList.get(mCurrentPosition).path;
            video = mVideoList.get(0);
            CommonData.isLastVideoFile = true;
          } else {
            mCurrentPosition = i + 1;
            mCurrentPath = mVideoList.get(mCurrentPosition).path;
            video = mVideoList.get(mCurrentPosition);
          }
          break;
        }
      }
      isListChanged = false;
    } else {
      mCurrentPosition += 1;
      if (mCurrentPosition >= mVideoList.size()) {
        mCurrentPosition = 0;
        CommonData.isLastVideoFile = true;
      }
      mCurrentPath = mVideoList.get(mCurrentPosition).path;
      video = mVideoList.get(mCurrentPosition);
    }

    return video;
  }

  public VideoFile getPreviousVideo() {
    VideoFile video = null;
    checkVideoList();
    if (isListChanged) {
      for (int i = 0; i < mVideoList.size(); i++) {
        if (mVideoList.get(i).path.equals(mCurrentPath)) {
          if (i - 1 < 0) {
            mCurrentPosition = mVideoList.size() - 1;
            mCurrentPath = mVideoList.get(mCurrentPosition).path;
            video = mVideoList.get(mCurrentPosition);
          } else {
            mCurrentPosition = i - 1;
            mCurrentPath = mVideoList.get(mCurrentPosition).path;
            video = mVideoList.get(i - 1);
          }
          break;
        }
        isListChanged = false;
      }
    } else {
      mCurrentPosition -= 1;
      if (mCurrentPosition < 0) {
        mCurrentPosition = mVideoList.size() - 1;
      }
      mCurrentPath = mVideoList.get(mCurrentPosition).path;
      video = mVideoList.get(mCurrentPosition);
    }

    return video;
  }

  public VideoFile getRandomVideo() {
    VideoFile video = null;
    checkVideoList();
    Random random = new Random();
    mCurrentPosition = random.nextInt(mVideoList.size());
    mCurrentPath = mVideoList.get(mCurrentPosition).path;
    video = mVideoList.get(mCurrentPosition);
    return video;
  }

  private void checkVideoList() {
    if (mVideoList == null || mVideoList.isEmpty()) {
      mVideoList = new ArrayList<VideoFile>(CommonData.getVideoList());
      isListChanged = true;
    } else {
      isListChanged = false;
    }
    return;
  }

  @Override
  public int describeContents() {
    Log.i(TAG, "--------describeContents---------");
    return 0;
  }

  @Override
  public void writeToParcel(Parcel vp, int vflag) {
    Log.i(TAG, "--------writeToParcel---------");

    vp.writeString(this.mCurrentPath);
    vp.writeInt(this.mCurrentPosition);
  }

  public static final Parcelable.Creator<VideoPlayList> CREATOR = new Creator<VideoPlayList>() {

    @Override
    public VideoPlayList[] newArray(int size) {
      Log.i("Parcelable.Creator", "--------newArray---------size: " + size);
      return new VideoPlayList[size];
    }

    @Override
    public VideoPlayList createFromParcel(Parcel source) {
      Log.i("Parcelable.Creator", "--------createFromParcel---------source:"
          + source);

      String path = source.readString();
      int index = source.readInt();

      VideoPlayList list = new VideoPlayList(path, index);
      list.setList();

      return list;
    }
  };

}
