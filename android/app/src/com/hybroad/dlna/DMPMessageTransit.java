package com.hybroad.dlna;

import com.hybroad.media.Device;
import com.hybroad.media.DevicesList;
import com.hybroad.util.PublicConstants;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class DMPMessageTransit extends BroadcastReceiver {
    private OnCurrentDMSChanged mOnCurrentDMSChanged;

    public DMPMessageTransit(OnCurrentDMSChanged onCurrentDMSChanged) {
        this.mOnCurrentDMSChanged = onCurrentDMSChanged;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (null == DevicesList.s_currentDevice
                || DevicesList.s_currentDevice.isLocalDevice()
                || null == DevicesList.s_currentDevice.getDeviceID())
            return;

        String action = intent.getAction();
        if (action.equals(PublicConstants.DMPIntent.ACTION_DMS_DEVICE_CHANGE)) {
            boolean isCurrentDeviceDown = true;
            for (Device device : DevicesList.s_devicesList) {
                if (device.isDMSDevice()
                        && DevicesList.s_currentDevice.getDeviceID().equals(
                                device.getDeviceID())
                        && DevicesList.s_currentDevice.getName().equals(
                                device.getName())) {
                    isCurrentDeviceDown = false;
                    break;
                }
            }
            if (isCurrentDeviceDown && null != mOnCurrentDMSChanged) {
                mOnCurrentDMSChanged.onDMSDeviceDown();
            }
        }
    }

}
