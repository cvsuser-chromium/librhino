package com.hybroad.video;

import java.io.Serializable;
import java.util.Comparator;

import com.hybroad.util.PinyinUtil;

/** Getter or setter is not recommended for efficiency. */
public class VideoFile implements Serializable {
  private static final long serialVersionUID = 1L;

  private int type; // 0:video 1:dir

  public int id; // no use
  public String path = "";
  public String title = "";
  public String mimeType = "";
  public long dateTime;
  public long size;

  public String objectID = ""; // for DMSFile
  public String classID = ""; // for DMSFile
  public String parentID = ""; // for DMSFile

  public boolean isVideo() {
    return (0 == type) ? true : false;
  }

  public boolean isFolder() {
    return (1 == type) ? true : false;
  }

  public void setTypeVideo() {
    this.type = 0;
  }

  public void setTypeFolder() {
    this.type = 1;
  }

  private transient Comparator<VideoFile> mDateTimeComparator = new Comparator<VideoFile>() {
    @Override
    public int compare(VideoFile arg0, VideoFile arg1) {
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

  public Comparator<VideoFile> getDateTimeComparator() {
    return this.mDateTimeComparator;
  }

  private transient Comparator<VideoFile> mTitleComparator = new Comparator<VideoFile>() {
    @Override
    public int compare(VideoFile arg0, VideoFile arg1) {
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

  public Comparator<VideoFile> getTitleComparator() {
    return this.mTitleComparator;
  }

  private transient Comparator<VideoFile> mSizeComparator = new Comparator<VideoFile>() {
    @Override
    public int compare(VideoFile arg0, VideoFile arg1) {
      int result = 0;
      if (arg0.size > arg1.size) {
        result = 1;
      } else if (arg0.size < arg1.size) {
        result = -1;
      }
      return result;
    }
  };

  public Comparator<VideoFile> getSizeComparator() {
    return this.mSizeComparator;
  }
}
