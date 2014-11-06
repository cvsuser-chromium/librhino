package com.hybroad.view.gif;

public interface GifAction {

  /**
   * get parser observer
   * 
   * @param parseStatus
   *          if the parser is success the value is true
   * @param frameIndex
   *          the step of the parser, when over the value is -1
   */
  public void parseOk(boolean parseStatus, int frameIndex);
}
