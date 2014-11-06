package com.hybroad.util;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.app.ActivityManager;
import android.content.Context;

public class ProcessUtils {

  public static void killProcessByPackage(Context context, String pkgName) {
    ActivityManager activityManager = (ActivityManager) context
        .getSystemService("activity");
    Method forceStopPackage = null;
    try {
      forceStopPackage = activityManager.getClass().getDeclaredMethod(
          "forceStopPackage", String.class);
      forceStopPackage.setAccessible(true);
      forceStopPackage.invoke(activityManager, pkgName);
    } catch (NoSuchMethodException e) {
      e.printStackTrace();
    } catch (IllegalArgumentException e) {
      e.printStackTrace();
    } catch (IllegalAccessException e) {
      e.printStackTrace();
    } catch (InvocationTargetException e) {
      e.printStackTrace();
    }
  }

  public static void killBackgroundProcesses(Context context, String packageName) {
    ActivityManager manager = (ActivityManager) context
        .getSystemService(Context.ACTIVITY_SERVICE);
    manager.killBackgroundProcesses(packageName);
  }
}
