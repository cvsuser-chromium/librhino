package com.hybroad.dlna;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.util.Log;

import com.hybroad.util.PublicConstants;
import com.hybroad.util.NetworkUtils;

public class DMRStarter extends BroadcastReceiver {
    private final String TAG = "---DMRStarter---";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, "onReceive() begin, action = " + action);

        action = intent.getAction();
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            Log.d(TAG,
                    "BOOT COMPLETED! To create DMRClient,Network is disconnected");
            if (NetworkUtils.isNetworkConnected(context)) {
                Log.d(TAG,
                        "BOOT COMPLETED! To create DMRClient, Network is connected");
                context.startService(new Intent(
                        PublicConstants.DLNACommon.DMR_JAVA_SERVICE_NAME));
                context.startService(new Intent(
                        "com.hybroad.dlna.AIRPLAY_SERVICE"));
            }
        } else if (ConnectivityManager.CONNECTIVITY_ACTION.equals(action)) {
            boolean isFailOver_connection = intent
                    .hasExtra(ConnectivityManager.CONNECTIVITY_ACTION);

            Log.d(TAG, "the network connection = " + isFailOver_connection);
            if (NetworkUtils.isNetworkConnected(context)) {
                Log.d(TAG,
                        "Network is connected! To confirm DMRClient is alive...");
                context.startService(new Intent(
                        PublicConstants.DLNACommon.DMR_JAVA_SERVICE_NAME));
                context.startService(new Intent(
                        "com.hybroad.dlna.AIRPLAY_SERVICE"));
            }
        } else if (PublicConstants.DMRIntent.ACTION_START_DMR.equals(action)) {
            Log.d(TAG, "MyMedia is started! To confirm DMRClient is alive...");
            context.startService(new Intent(
                    PublicConstants.DLNACommon.DMR_JAVA_SERVICE_NAME));
            context.startService(new Intent("com.hybroad.dlna.AIRPLAY_SERVICE"));
        }
        Log.d(TAG, "onReceive end");
    }

}
