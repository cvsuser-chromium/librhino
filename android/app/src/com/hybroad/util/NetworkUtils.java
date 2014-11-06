package com.hybroad.util;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.wifi.WifiManager;
import android.util.Log;

public class NetworkUtils extends BroadcastReceiver {
  private final static String TAG = "NetworkUtils";

  public static int getNetworkState(Context context) {
    ConnectivityManager connectManager = (ConnectivityManager) context
        .getSystemService(Context.CONNECTIVITY_SERVICE);
    if (connectManager == null) {
      return PublicConstants.NetworkState.DISCONNECTED;
    }
    NetworkInfo ethernetInfo = connectManager
        .getNetworkInfo(ConnectivityManager.TYPE_ETHERNET);
    NetworkInfo wifiInfo = connectManager
        .getNetworkInfo(ConnectivityManager.TYPE_WIFI);
    WifiManager wifiManager = (WifiManager) context
        .getSystemService(Context.WIFI_SERVICE);
    if (ethernetInfo != null && ethernetInfo.isAvailable()
        && ethernetInfo.isConnected()
        && ethernetInfo.getState() == State.CONNECTED) {
      return PublicConstants.NetworkState.ETHERNET_CONNECTED;
    } else if (wifiManager != null
        && wifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED
        && wifiInfo != null && wifiInfo.isAvailable()
        && wifiInfo.getState() == State.CONNECTED) {
      return PublicConstants.NetworkState.WIFI_FULL;
    } else {
      return PublicConstants.NetworkState.DISCONNECTED;
    }
  }

  public static boolean isNetworkConnected(Context context) {
    return !(PublicConstants.NetworkState.DISCONNECTED == getNetworkState(context));
  }

  @Override
  public void onReceive(Context context, Intent intent) {
    Log.e(TAG, "Network changed...");
    if (NetworkUtils.isNetworkConnected(context)) {
      Log.d(TAG, "Network is connected! To confirm DMRClient is alive...");
      context.startService(new Intent(
          PublicConstants.DLNACommon.DMR_JAVA_SERVICE_NAME));
    }
  }
}
