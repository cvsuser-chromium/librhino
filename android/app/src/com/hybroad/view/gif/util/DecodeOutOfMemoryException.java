package com.hybroad.view.gif.util;

public class DecodeOutOfMemoryException extends DecodeException {
  /**
   * set the serialVersionUID
   */
  private static final long serialVersionUID = -6505852491970510267L;

  public DecodeOutOfMemoryException() {
    super("out of memory");
  }

  public DecodeOutOfMemoryException(String msg) {
    super(msg);
  }
}