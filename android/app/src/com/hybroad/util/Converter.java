package com.hybroad.util;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.DecimalFormat;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.util.Log;

public class Converter {
  private final static String TAG = "----Converter----";
  private final static int BUFFER_SIZE = 4096;

  public static String convertSpecialCharacters(String name) {
    if (name == null) {
      return null;
    }

    if (name.contains("%")) {
      name = name.replace("%", "%25");
    }
    if (name.contains("#")) {
      name = name.replace("#", "%23");
    }
    if (name.contains("?")) {
      name = name.replace("?", "%3F");
    }
    return name;
  }

  /** convert '00:00:00' or '00:00:00.000' to millisecond */
  public static long convertHoursMinutesSecondsToMsec(String timeString) {
    Log.d(TAG, "convert timeString[ " + timeString + "] to millisecond");
    String timeArray[] = timeString.split(":");
    if (null == timeArray || timeArray.length != 3)
      return 0;

    if (!isInteger(timeArray[0]) || !isInteger(timeArray[1]))
      return 0;

    long millisecond = 0;
    if (timeArray[2].indexOf('.') != -1) {
      String secondArray[] = timeArray[2].split("\\.");
      if (null == secondArray || secondArray.length != 2)
        return 0;
      if (!isInteger(secondArray[0]) || !isInteger(secondArray[1]))
        return 0;

      int hour = Integer.parseInt(timeArray[0]);
      int minute = Integer.parseInt(timeArray[1]);
      int second = Integer.parseInt(secondArray[0]);
      millisecond = ((hour * 60 + minute) * 60 + second) * 1000
          + Integer.parseInt(secondArray[1]);
    } else {
      if (!isInteger(timeArray[2]))
        return 0;

      int hour = Integer.parseInt(timeArray[0]);
      int minute = Integer.parseInt(timeArray[1]);
      int second = Integer.parseInt(timeArray[2]);
      millisecond = ((hour * 60 + minute) * 60 + second) * 1000;
    }

    return millisecond;
  }

  public static String convertToHoursMinutesSeconds(long msec) {
    if (msec < 0) {
      return "00:00:00";
    }
    long t = msec / 1000;
    return MessageFormat.format("{0,number,00}:{1,number,00}:{2,number,00}",
        t / 60 / 60, t / 60 % 60, t % 60);
  }

  public static String convertToDateTimeString(long dateTime, int style) {
    Date date = new Date(dateTime);
    SimpleDateFormat format;
    switch (style) {
    case 0:
      format = new SimpleDateFormat("yyyy-MM-dd/kk:mm:ss");
      break;

    case 1:
      format = new SimpleDateFormat("yyyy-MM-dd kk:mm:ss");
      break;

    case 2:
      format = new SimpleDateFormat("dd/MM/yyyy kk:mm:ss");
      break;

    default:
      format = new SimpleDateFormat("yyyy-MM-dd/kk:mm:ss");
      break;
    }
    return format.format(date);
  }

  public static String bytesSizeConverter(long fileSize) {
    DecimalFormat decimalFormat = new DecimalFormat("#.00");
    String fileSizeString = "";
    if (fileSize < 1024) {
      fileSizeString = decimalFormat.format((double) fileSize) + "B";
    } else if (fileSize < 1048576) {
      fileSizeString = decimalFormat.format((double) fileSize / 1024) + "KB";
    } else if (fileSize < 1073741824) {
      fileSizeString = decimalFormat.format((double) fileSize / 1048576) + "MB";
    } else {
      fileSizeString = decimalFormat.format((double) fileSize / 1073741824)
          + "GB";
    }
    return fileSizeString;
  }

  /**
   * 
   * @param str
   *          the string to be examined.
   * @return true if str is Integer or not.
   */
  public static boolean isInteger(String str) {
    Pattern pattern = Pattern.compile("^[-\\+]?[\\d]+$");
    Matcher matcher = pattern.matcher((CharSequence) str);
    return matcher.find();
  }

  public static byte[] InputStreamToByte(InputStream is) {
    ByteArrayOutputStream byteArrOut = new ByteArrayOutputStream();
    byte[] temp = new byte[BUFFER_SIZE];
    int len = 0;
    try {
      while ((len = is.read(temp, 0, BUFFER_SIZE)) != -1) {
        byteArrOut.write(temp, 0, len);
      }

      byte[] bytes = byteArrOut.toByteArray();
      Log.d(TAG, "bytes = " + bytes);
      byteArrOut.flush();
      return bytes;
    } catch (IOException ioException) {
      Log.e(TAG, "IOException: ", ioException);
    }

    return null;
  }
}
