package com.hybroad.video;

import java.util.List;

import android.os.Parcelable;

/**
 * Play list manager interface
 * 
 * @author xiongCuiFan
 * 
 */
public abstract class MediaFileList implements Parcelable {

  /**
   * start player from where 0 - play list 1 - FileM
   */
  private int id;

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  /**
   * get previous video
   * 
   * @return
   */
  public abstract VideoFile getPreVideoInfo(List<VideoFile> list);

  /**
   * get next video
   * 
   * @return
   */
  public abstract VideoFile getNextVideoInfo(List<VideoFile> list);

  /**
   * get next video in mode all no cycle
   * 
   * @return
   */
  public abstract VideoFile getNextVideoInfo_NoCycle(List<VideoFile> list);

  /**
   * get random video
   * 
   * @return
   */
  public abstract VideoFile getRandomVideoInfo(List<VideoFile> list);

  /**
   * get current video
   * 
   * @return
   */
  public abstract VideoFile getCurrVideoInfo();

}
