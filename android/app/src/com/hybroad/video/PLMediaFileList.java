package com.hybroad.video;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * play list control child class
 * 
 * @author xiongCuiFan
 */
public class PLMediaFileList extends MediaFileList implements Parcelable {

  private String TAG = "PLMediaFileList";
  private ArrayList<VideoFile> mediaList = null;
  private String currPath = null;
  private int currPosition = 0;
  private boolean isListChanged = false;

  public PLMediaFileList(String currPath, int currPosition) {

    this.currPath = currPath;
    this.currPosition = currPosition;
    setId(VideoConstant.FROM_INTERNAL_FILE_MANAGER);

  }

  /**
   * get play list
   * 
   * @author xiongCUiFan
   */
  public void setList() {
    this.mediaList = (ArrayList<VideoFile>) CommonData.getVideoList();
  }

  @Override
  public VideoFile getCurrVideoInfo() {
    Log.i(TAG, "--------getCurrVideoInfo-----------mediaList:" + mediaList
        + " currPosition:" + currPosition);
    if (mediaList == null || currPosition < 0
        || currPosition >= mediaList.size()) {
      return null;
    }
    return mediaList.get(currPosition);
  }

  @Override
  public VideoFile getNextVideoInfo(List<VideoFile> list) {

    VideoFile model = null;
    checkMediaList();
    Log.i(TAG, "-------getNextVideoInfo-------isListChanged:" + isListChanged);
    if (isListChanged) {
      for (int i = 0; i < mediaList.size(); i++) {
        if (mediaList.get(i).path.equals(currPath)) {
          if (i + 1 >= mediaList.size()) {
            currPosition = 0;
            currPath = mediaList.get(currPosition).path;
            model = mediaList.get(0);
            break;
          } else {
            currPosition = i + 1;
            currPath = mediaList.get(currPosition).path;
            model = mediaList.get(currPosition);
            break;
          }
        }
      }
      isListChanged = false;
    } else {
      currPosition += 1;
      if (currPosition >= mediaList.size()) {
        currPosition = 0;
      }
      currPath = mediaList.get(currPosition).path;
      model = mediaList.get(currPosition);
    }

    return model;
  }

  @Override
  public VideoFile getNextVideoInfo_NoCycle(List<VideoFile> list) {

    VideoFile model = null;
    checkMediaList();
    Log.i(TAG, "-------getNextVideoInfo-------isListChanged:" + isListChanged);
    if (isListChanged) {
      for (int i = 0; i < mediaList.size(); i++) {
        if (mediaList.get(i).path.equals(currPath)) {
          if (i + 1 >= mediaList.size()) {
            currPosition = 0;
            currPath = mediaList.get(currPosition).path;
            model = mediaList.get(0);
            // CommonData.isLastMediaFile = true;
            break;
          } else {
            currPosition = i + 1;
            currPath = mediaList.get(currPosition).path;
            model = mediaList.get(currPosition);
            break;
          }
        }
      }
      isListChanged = false;
    } else {
      currPosition += 1;
      if (currPosition >= mediaList.size()) {
        currPosition = 0;
        // CommonData.isLastMediaFile = true;
      }
      currPath = mediaList.get(currPosition).path;
      model = mediaList.get(currPosition);
    }

    return model;
  }

  @Override
  public VideoFile getPreVideoInfo(List<VideoFile> list) {
    VideoFile model = null;
    checkMediaList();
    if (isListChanged) {
      for (int i = 0; i < mediaList.size(); i++) {
        if (mediaList.get(i).path.equals(currPath)) {
          if (i - 1 < 0) {
            currPosition = mediaList.size() - 1;
            currPath = mediaList.get(currPosition).path;
            model = mediaList.get(currPosition);
            break;
          } else {
            currPosition = i - 1;
            currPath = mediaList.get(currPosition).path;
            model = mediaList.get(i - 1);
            break;
          }
        }
        isListChanged = false;
      }
    } else {
      currPosition -= 1;
      if (currPosition < 0) {
        currPosition = mediaList.size() - 1;
      }
      currPath = mediaList.get(currPosition).path;
      model = mediaList.get(currPosition);
    }

    return model;
  }

  @Override
  public VideoFile getRandomVideoInfo(List<VideoFile> list) {
    VideoFile model = null;
    checkMediaList();
    Random random = new Random();
    currPosition = random.nextInt(mediaList.size());
    currPath = mediaList.get(currPosition).path;
    model = mediaList.get(currPosition);
    return model;
  }

  /**
   * check play list
   * 
   * @author xiongCuiFan
   */
  private void checkMediaList() {
    if (mediaList == null || mediaList.isEmpty()) {
      mediaList = new ArrayList<VideoFile>(CommonData.getVideoList());
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

    vp.writeString(this.currPath);
    vp.writeInt(this.currPosition);
  }

  public static final Parcelable.Creator<PLMediaFileList> CREATOR = new Creator<PLMediaFileList>() {

    @Override
    public PLMediaFileList[] newArray(int size) {
      Log.i("Parcelable.Creator", "--------newArray---------");

      return new PLMediaFileList[size];
    }

    @Override
    public PLMediaFileList createFromParcel(Parcel source) {
      Log.i("Parcelable.Creator", "--------createFromParcel---------source:"
          + source);

      String pth = source.readString();
      int idx = source.readInt();

      PLMediaFileList r = new PLMediaFileList(pth, idx);
      r.setList();

      return r;
    }
  };

}
