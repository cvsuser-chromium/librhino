package com.hybroad.video;

import java.util.List;

public class CommonData {

  /* whether the resume function has been called. */
  public static boolean isResume = false;

  /* whether the current file is the last media file in order mode. */
  public static boolean isLastVideoFile = false;

  private static List<VideoFile> sVideoList = null;

  public static synchronized List<VideoFile> getVideoList() {
    return CommonData.sVideoList;
  }

  public static synchronized void setVideoList(List<VideoFile> videoList) {
    CommonData.sVideoList = videoList;
  }

}
