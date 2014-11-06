package com.hybroad.media;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.ethernet.EthernetManager;
import android.net.wifi.WifiManager;
import android.util.Log;

import com.hybroad.util.NetworkUtils;

public class NetworkStateListener extends BroadcastReceiver {
	private OnNetworkStateChanged mOnNetworkStateChanged;

	public NetworkStateListener(OnNetworkStateChanged onNetworkStateChanged) {
		mOnNetworkStateChanged = onNetworkStateChanged;
	}

	@Override
    public void onReceive(Context context, Intent intent) {
        if (null == mOnNetworkStateChanged) {
            return;
        }
        final String action = intent.getAction();
        Log.d("NetworkStateListener", "onReceive action = " + action);
//        if (action.equals(WifiManager.RSSI_CHANGED_ACTION) || action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)
//                || action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)
//                || action.equals(EthernetManager.ETHERNET_STATE_CHANGED_ACTION)
//                || action.equals(EthernetManager.NETWORK_STATE_CHANGED_ACTION)
//                || action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
//            mOnNetworkStateChanged.onChanged(NetworkUtils.getNetworkState(context));
//        }
        if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
            mOnNetworkStateChanged.onChanged(NetworkUtils.getNetworkState(context));
        }
    }
}
