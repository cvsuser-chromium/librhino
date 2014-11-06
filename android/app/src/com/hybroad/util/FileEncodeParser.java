package com.hybroad.util;

import java.io.File;
import java.net.URL;

import android.util.Log;

import info.monitorenter.cpdetector.io.ASCIIDetector;
import info.monitorenter.cpdetector.io.CodepageDetectorProxy;
import info.monitorenter.cpdetector.io.JChardetFacade;
import info.monitorenter.cpdetector.io.ParsingDetector;
import info.monitorenter.cpdetector.io.UnicodeDetector;

public class FileEncodeParser {
  private final static String TAG = "----FileEncodeParser----";

  public static String getFileEncode(String path) {
    if (path == null) {
      return null;
    }

    CodepageDetectorProxy detector = CodepageDetectorProxy.getInstance();
    detector.add(new ParsingDetector(false));
    detector.add(JChardetFacade.getInstance());
    detector.add(ASCIIDetector.getInstance());
    detector.add(UnicodeDetector.getInstance());
    java.nio.charset.Charset charset = null;
    File f = new File(path);

    try {
      charset = detector.detectCodepage(f.toURI().toURL());
    } catch (Exception ex) {
      ex.printStackTrace();
      return null;
    }

    if (charset != null) {
      Log.i(TAG, "The charset name of " + path + " is " + charset.name());
      return charset.name();
    } else {
      Log.i(TAG, "Cannot parse charset of " + path);
      return null;
    }
  }

  public static String getFileEncode(URL url) {
    if (url == null) {
      return null;
    }

    CodepageDetectorProxy detector = CodepageDetectorProxy.getInstance();
    detector.add(new ParsingDetector(false));
    detector.add(JChardetFacade.getInstance());
    detector.add(ASCIIDetector.getInstance());
    detector.add(UnicodeDetector.getInstance());
    java.nio.charset.Charset charset = null;

    try {
      charset = detector.detectCodepage(url);
    } catch (Exception ex) {
      ex.printStackTrace();
    }

    if (charset != null) {
      Log.i(TAG, "The charset name of " + url + " is " + charset.name());
      return charset.name();
    } else {
      Log.i(TAG, "Cannot parse charset of " + url);
      return null;
    }
  }

}
