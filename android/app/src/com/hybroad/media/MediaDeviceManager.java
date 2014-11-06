package com.hybroad.media;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.ethernet.EthernetManager;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.AdapterView;
import android.widget.GridView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import com.hybroad.dlna.DMPDevice;
import com.hybroad.dlna.DMPDevice.OnDMSListChangedListener;
import com.hybroad.dlna.DMPDevice.OnFileListChangedListener;
import com.hybroad.dlna.GlobalDMPDevice;
import com.hybroad.dlna.LibraryLoader;
import com.hybroad.json.DMSDeviceJSON;
import com.hybroad.json.DMSDeviceListJSON;
import com.hybroad.json.JSONUtils;
import com.hybroad.util.CustomWidget;
import com.hybroad.util.NetworkUtils;
import com.hybroad.util.ProcessUtils;
import com.hybroad.util.PublicConstants;
import com.hybroad.util.PublicConstants.ActivityState;
import com.hybroad.util.StatusBarOperator;

public class MediaDeviceManager extends Activity {
  private final String TAG = "----MediaDeviceManager----";

  private Context mContext;
  public static boolean isAppStartedWithNormalMode = false;

  private StorageManager mStorageManager;
  private NetworkStateListener mNetworkStateListener;

  private boolean mFirstGetNetState = true;
  private int mActivityState = ActivityState.UNKNOWN;
  private DLNAServiceRebootReceiver mDLNAServiceRebootReceiver;
  private boolean mFirstCreateDMPDevice = true;

  private Timer mTimer;
  private TimerTask mTimerTask;

  /** Called when the activity is first created. */
  public void onCreate(Bundle savedInstanceState) {
    Log.i(TAG, "----onCreate()----");
    super.onCreate(savedInstanceState);
    mActivityState = ActivityState.ON_CREATE;
    mContext = this;
    isAppStartedWithNormalMode = true;
    StatusBarOperator.hideStatusBar(this);
    setContentView(R.layout.device_manager_main);

    CustomWidget.showLoadIcon(mContext);
    new Thread(new searchDevicesRunnable()).start();

    registerNetworkStateListener();

    Intent intent = new Intent(PublicConstants.DMRIntent.ACTION_START_DMR);
    sendBroadcast(intent);

    mDLNAServiceRebootReceiver = new DLNAServiceRebootReceiver();
    IntentFilter filter = new IntentFilter();
    filter.addAction(PublicConstants.DMRIntent.ACTION_DLNA_SERVER_DIED);
    filter.addAction(PublicConstants.DMRIntent.ACTION_DLNA_SERVER_STARTED);
    registerReceiver(mDLNAServiceRebootReceiver, filter);

    // startTimerTask();

    // kill one of 'android.process.media'
    // ProcessUtils.killProcessByPackage(this, "com.android.providers.media");
  }

  @Override
  protected void onStart() {
    Log.d(TAG, "----onStart----");
    super.onStart();
    mActivityState = ActivityState.ON_START;
  }

  @Override
  protected void onResume() {
    Log.d(TAG, "----onResume----");
    super.onResume();
    mActivityState = ActivityState.ON_RESUME;
  }

  @Override
  protected void onPause() {
    Log.d(TAG, "----onPause----");
    super.onPause();
    mActivityState = ActivityState.ON_PAUSE;
  }

  @Override
  protected void onStop() {
    Log.d(TAG, "----onStop----");
    super.onStop();
    CustomWidget.releaseLoadIcon();
    mActivityState = ActivityState.ON_STOP;
  }

  protected void onDestroy() {
    Log.d(TAG, "----onDestroy----");
    super.onDestroy();
    mActivityState = ActivityState.ON_DESTROY;
    isAppStartedWithNormalMode = false;

    // stopTimerTask();

    if (null != mStorageManager) {
//      mStorageManager.unregisterListener(mStorageEventListener);
    }
    if (null != mNetworkStateListener) {
      unregisterReceiver(mNetworkStateListener);
    }

    if (null != mDLNAServiceRebootReceiver) {
      unregisterReceiver(mDLNAServiceRebootReceiver);
    }

    Log.d(TAG, "to release DMPDevice");
    if (null != GlobalDMPDevice.s_DMPDevice) {
      GlobalDMPDevice.s_DMPDevice.release();
      GlobalDMPDevice.s_DMPDevice = null;
    }
    Log.d(TAG, "release DMPDevice ok");

    clearStaticVariable();

    // kill one of 'android.process.media'
    // ProcessUtils.killProcessByPackage(this, "com.android.providers.media");
  }

  private Handler updateThumbnailLayoutHandler = new Handler() {
    public void handleMessage(Message msg) {
      CustomWidget.hideLoadIcon();
      updateDeviceBrower();
    }
  };

  private void registerNetworkStateListener() {
    mNetworkStateListener = new NetworkStateListener(
        new OnNetworkStateChanged() {
          @Override
          public void onChanged(int connectState) {
            switch (connectState) {
            case PublicConstants.NetworkState.DISCONNECTED:
              Log.d(TAG, "...NetworkState: DISCONNECTED...");
              CustomWidget.toastNetworkDisconnect(mContext);
              if (null == DevicesList.s_devicesList) {
                break;
              }
              for (int i = 0; i < DevicesList.s_devicesList.size(); i++) {
                if (DevicesList.s_devicesList.get(i).isDMSDevice()) {
                  DevicesList.s_devicesList.remove(i);
                  i--;
                }
              }
              updateThumbnailLayoutHandler.sendEmptyMessage(0);
              notifyDMSChanged();
              break;

            case PublicConstants.NetworkState.ETHERNET_CONNECTED:
              Log.d(TAG, "...NetworkState: ETHERNET_CONNECTED...");
              if (mFirstGetNetState) {
                mFirstGetNetState = false;
              } else {
                if (mActivityState == ActivityState.ON_RESUME) {
                  CustomWidget.showLoadIcon(mContext);
                }
                new Thread(new searchDevicesRunnable()).start();
              }
              break;

            case PublicConstants.NetworkState.WIFI_FULL:
              Log.d(TAG, "...NetworkState: WIFI_FULL...");
              if (mFirstGetNetState) {
                mFirstGetNetState = false;
              } else {
                if (mActivityState == ActivityState.ON_RESUME) {
                  CustomWidget.showLoadIcon(mContext);
                }
                new Thread(new searchDevicesRunnable()).start();
              }
              break;

            default:
              break;
            }
          }
        });
    IntentFilter filter = new IntentFilter(WifiManager.RSSI_CHANGED_ACTION);
    filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
    filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
    filter.addAction(EthernetManager.ETHERNET_STATE_CHANGED_ACTION);
    filter.addAction(EthernetManager.NETWORK_STATE_CHANGED_ACTION);
    filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
    filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
    filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
    registerReceiver(mNetworkStateListener, filter);
  }

  private void notifyDMSChanged() {
    Intent intent = new Intent(
        PublicConstants.DMPIntent.ACTION_DMS_DEVICE_CHANGE);
    sendBroadcast(intent);
  }

  private void clearStaticVariable() {
    if (DevicesList.s_devicesList != null) {
      DevicesList.s_devicesList.clear();
      DevicesList.s_devicesList = null;
    }
    DevicesList.s_currentDevice = null;
  }

  final StorageEventListener mStorageEventListener = new StorageEventListener() {
    @Override
    public void onStorageStateChanged(String path, String oldState,
        String newState) {
      super.onStorageStateChanged(path, oldState, newState);

      if (mActivityState == ActivityState.ON_RESUME) {
        CustomWidget.showLoadIcon(mContext);
      }

      if (newState.equals("mounted")) {
        if (DevicesList.s_devicesList == null) {
          DevicesList.s_devicesList = new ArrayList<Device>();
        }
        Device device = new Device();
        device.setRootPath(path);
        Log.d(TAG, "new device Path: " + device.getRootPath());
        int position = device.getRootPath().lastIndexOf('/') + 1; // start at
                                                                  // last '/' of
                                                                  // "/mnt/sda/sda1"
        if (position >= 0 && position < device.getRootPath().length()) {
          device.setName(device.getRootPath().substring(position));
        }
        Log.d(TAG, "new device name: " + device.getName());
        device.setTypeLocal();
        DevicesList.s_devicesList.add(device);
      } else if (newState.equals("unmounted")) {
        if (DevicesList.s_devicesList != null) {
          for (int i = 0; i < DevicesList.s_devicesList.size(); i++) {
            if (DevicesList.s_devicesList.get(i).isLocalDevice()) {
              if (path.equals(DevicesList.s_devicesList.get(i).getRootPath())) { // list
                                                                                 // maybe
                                                                                 // clear
                                                                                 // up
                                                                                 // by
                                                                                 // other
                                                                                 // thread!!!
                DevicesList.s_devicesList.remove(i);
                break;
              }
            }
          }
        }
      }

      updateThumbnailLayoutHandler.sendEmptyMessage(0);
    }
  };

  private void updateDeviceBrower() {
    if (DevicesList.s_devicesList != null
        && !DevicesList.s_devicesList.isEmpty()) {
      switchDeviceThumbnails();
    } else {
      switchNoDeviceNotice();
    }
  }

  private void switchDeviceThumbnails() {
    LinearLayout deviceThumbnailsLayout = (LinearLayout) ((Activity) mContext)
        .getLayoutInflater().inflate(R.layout.device_manager_thumbnail, null);
    RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
        LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
    mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
    deviceThumbnailsLayout.setLayoutParams(mLayoutParams);

    GridView devicesGridviewTemp = (GridView) deviceThumbnailsLayout
        .findViewById(R.id.device_gridview);
    devicesGridviewTemp.setSelector(R.drawable.selector_1);
    DeviceThumbItemAdapter deviceThumbItemAdapter = new DeviceThumbItemAdapter(
        mContext, DevicesList.s_devicesList);
    devicesGridviewTemp.setAdapter(deviceThumbItemAdapter);
    devicesGridviewTemp
        .setOnItemClickListener(new GridView.OnItemClickListener() {
          @Override
          public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
              long arg3) {
            if (DevicesList.s_devicesList != null && arg2 >= 0
                && arg2 < DevicesList.s_devicesList.size()) {
              DevicesList.s_currentDevice = DevicesList.s_devicesList.get(arg2);
              Intent deviceIntent = new Intent();
              deviceIntent.setClass(mContext, MediaFileManager.class);
              startActivity(deviceIntent);
            }
          }
        });

    RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.device_list_mainarea);
    mainArea.removeAllViews();
    mainArea.addView(deviceThumbnailsLayout);
    GridView mDevicesGridview = (GridView) mainArea
        .findViewById(R.id.device_gridview);
    mDevicesGridview.requestFocus();
    mDevicesGridview.setSelection(0);
  }

  private void switchNoDeviceNotice() {
    LinearLayout noDeviceLayout = (LinearLayout) ((Activity) mContext)
        .getLayoutInflater().inflate(R.layout.device_manager_no_device, null);
    RelativeLayout.LayoutParams mLayoutParams = new RelativeLayout.LayoutParams(
        LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
    mLayoutParams.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
    noDeviceLayout.setLayoutParams(mLayoutParams);

    RelativeLayout mainArea = (RelativeLayout) findViewById(R.id.device_list_mainarea);
    mainArea.removeAllViews();
    mainArea.addView(noDeviceLayout);
  }

  private void searchLocalDevices() {
    if (DevicesList.s_devicesList == null) {
      DevicesList.s_devicesList = new ArrayList<Device>();
    }
    if (null == mStorageManager) {
      mStorageManager = (StorageManager) mContext
          .getSystemService(STORAGE_SERVICE);
//      mStorageManager.registerListener(mStorageEventListener);
    }
//    String volumePaths[] = mStorageManager.getVolumePaths();
//    if (volumePaths != null && volumePaths.length > 0) {
//      for (int i = 0; i < volumePaths.length; i++) {
//        if ("/root".equals(volumePaths[i])
//            || "/mnt/nand".equals(volumePaths[i])
//            || volumePaths[i].startsWith("/sdcard")
//            || volumePaths[i].startsWith("/mnt/emmc")
//            || volumePaths[i].startsWith("/mnt/mmc")) {
//          continue;
//        }
//        if (volumePaths[i] == null || volumePaths[i].isEmpty()) {
//          continue;
//        }
//        Device device = new Device();
//        device.setRootPath(volumePaths[i]);
//        Log.d(TAG, "new device Path: " + device.getRootPath());
//
//        // start at last '/' of "/mnt/sda/sda1"
//        int position = device.getRootPath().lastIndexOf('/') + 1;
//        if (position >= 0 && position < device.getRootPath().length()) {
//          device.setName(device.getRootPath().substring(position));
//        }
//        Log.d(TAG, "new device name: " + device.getName());
//        device.setTypeLocal();
//        DevicesList.s_devicesList.add(device);
//      }
//    }
  }

  private void searchDMSDevices() {
    if (DevicesList.s_devicesList == null) {
      DevicesList.s_devicesList = new ArrayList<Device>();
    }

    if (mFirstCreateDMPDevice || null == GlobalDMPDevice.s_DMPDevice) {
      if (mFirstCreateDMPDevice) {
        mFirstCreateDMPDevice = false;
      }
      Log.d(TAG, "to create DMPDevice");
      LibraryLoader.ensureInitialized();
      GlobalDMPDevice.s_DMPDevice = DMPDevice.create(mContext);
      if (null == GlobalDMPDevice.s_DMPDevice) {
        Log.d(TAG, "mDMPDevice is null");
        return;
      }
      GlobalDMPDevice.s_DMPDevice
          .setDMSListChangedListener(new OnDMSListChangedListener() {
            @Override
            public void onChanged(DMPDevice mp) {
              Log.d(TAG, "--DMS--------OnDMSListChangedListener--------- ");
              if (mActivityState == ActivityState.ON_RESUME) {
                CustomWidget.showLoadIcon(mContext);
              }
              new Thread(new searchDevicesRunnable()).start();
            }
          });
      GlobalDMPDevice.s_DMPDevice
          .setFileListChangedListener(new OnFileListChangedListener() {
            @Override
            public void onChanged(DMPDevice mp) {
              Log.d(TAG, "--DMS--------OnFileListChangedListener--------- ");
              Intent intent = new Intent(
                  PublicConstants.DMPIntent.ACTION_DMS_FILE_CHANGE);
              sendBroadcast(intent);
            }
          });
    }

    // first remove all dms device
    for (int i = 0; i < DevicesList.s_devicesList.size(); i++) {
      if (DevicesList.s_devicesList.get(i).isDMSDevice()) {
        DevicesList.s_devicesList.remove(i);
        i--;
      }
    }
    // Log.d(TAG, "to re-search Dms");
    // GlobalDMPDevice.s_DMPDevice.researchDms(true);

    Log.d(TAG, "to getDmsCount");
    int dmsCount = GlobalDMPDevice.s_DMPDevice.getDmsCount();
    Log.d(TAG, "dmsCount: " + dmsCount);

    String dmsListJson = GlobalDMPDevice.s_DMPDevice.getDmsList(0, dmsCount);
    Log.d(TAG, "dmsList: " + dmsListJson);

    DMSDeviceListJSON dmsListObject = new DMSDeviceListJSON();
    if (!TextUtils.isEmpty(dmsListJson)) {
      dmsListObject = (DMSDeviceListJSON) JSONUtils.loadObjectFromJSON(
          dmsListJson, dmsListObject);
    }
    ArrayList<DMSDeviceJSON> devices = null;
    if (dmsListObject != null) {
      Log.d(TAG, "------device_count: " + dmsListObject.getCount());
      if ((devices = dmsListObject.getDmsList()) != null) {
        for (DMSDeviceJSON deviceJSON : devices) {
          Device device = new Device();
          device.setDeviceID(deviceJSON.getDeviceID());
          Log.d(TAG, "------getDeviceID: " + device.getDeviceID());
          device.setName(deviceJSON.getFriendlyName());
          Log.d(TAG, "------getName: " + device.getName());
          device.setTypeDMS();
          DevicesList.s_devicesList.add(device);
        }
      }
    }
    notifyDMSChanged();
  }

  private class searchDevicesRunnable implements Runnable {
    @Override
    public void run() {
      if (DevicesList.s_devicesList != null) {
        DevicesList.s_devicesList.clear();
        DevicesList.s_devicesList = null;
      }

      searchLocalDevices();
      updateThumbnailLayoutHandler.sendEmptyMessage(0);

      if (NetworkUtils.isNetworkConnected(mContext)
          && "running".equals(android.os.SystemProperties
              .get(PublicConstants.DLNACommon.DLNA_BIN_SERVICE_STATE_PROPERTY))) {
        searchDMSDevices();
        updateThumbnailLayoutHandler.sendEmptyMessage(0);
      }
    }
  }

  private class DLNAServiceRebootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction();
      if (PublicConstants.DMRIntent.ACTION_DLNA_SERVER_DIED.equals(action)) {
        GlobalDMPDevice.s_DMPDevice = null;
        if (null == DevicesList.s_devicesList) {
          return;
        }
        for (int i = 0; i < DevicesList.s_devicesList.size(); i++) {
          if (DevicesList.s_devicesList.get(i).isDMSDevice()) {
            DevicesList.s_devicesList.remove(i);
            i--;
          }
        }
        updateThumbnailLayoutHandler.sendEmptyMessage(0);
        notifyDMSChanged();
      } else if (PublicConstants.DMRIntent.ACTION_DLNA_SERVER_STARTED
          .equals(action)) {
        if (mActivityState == ActivityState.ON_RESUME) {
          CustomWidget.showLoadIcon(mContext);
        }
        new Thread(new searchDevicesRunnable()).start();
      }
    }

  }

  private void startTimerTask() {
    if (mTimer == null) {
      mTimer = new Timer(true);
    }
    if (mTimerTask == null) {
      mTimerTask = new TimerTask() {
        public void run() {
          if (NetworkUtils.isNetworkConnected(mContext)
              && "running"
                  .equals(android.os.SystemProperties
                      .get(PublicConstants.DLNACommon.DLNA_BIN_SERVICE_STATE_PROPERTY))) {
            boolean isDMSExist = false;
            for (int i = 0; i < DevicesList.s_devicesList.size(); i++) {
              if (DevicesList.s_devicesList.get(i).isDMSDevice()) {
                isDMSExist = true;
                break;
              }
            }
            if (isDMSExist) {
              searchDMSDevices();
              updateThumbnailLayoutHandler.sendEmptyMessage(0);
            }
          }
        }
      };
    }
    mTimer.schedule(mTimerTask, 0, 45000);
  }

  private void stopTimerTask() {
    if (mTimerTask != null) {
      mTimerTask.cancel();
      mTimerTask = null;
    }
  }
}
