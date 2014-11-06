package com.hybroad.media;

import java.util.ArrayList;

import com.hybroad.media.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class DeviceThumbItemAdapter extends BaseAdapter {
    private Context mContext;
    private ArrayList<Device> mDevicesList;

    public DeviceThumbItemAdapter(Context context, ArrayList<Device> devicesList) {
        this.mContext = context;
        this.mDevicesList = devicesList;
    }

    @Override
    public int getCount() {
        // TODO Auto-generated method stub
        return (this.mDevicesList == null) ? 0 : this.mDevicesList.size();
    }

    @Override
    public Object getItem(int position) {
        // TODO Auto-generated method stub
        return (this.mDevicesList == null) ? null : this.mDevicesList
                .get(position);
    }

    @Override
    public long getItemId(int position) {
        // TODO Auto-generated method stub
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        // TODO Auto-generated method stub
        DeviceThumbItemLayout mDeviceThumbItemLayout = null;
        if (convertView == null) {
            mDeviceThumbItemLayout = new DeviceThumbItemLayout();
            convertView = LayoutInflater.from(mContext).inflate(
                    R.layout.device_manager_thumbnail_item, null);
            mDeviceThumbItemLayout.thumbImage = (ImageView) convertView
                    .findViewById(R.id.device_thumb_image);
            mDeviceThumbItemLayout.titleText = (TextView) convertView
                    .findViewById(R.id.device_thumb_title);
            convertView.setTag(mDeviceThumbItemLayout);
        } else {
            mDeviceThumbItemLayout = (DeviceThumbItemLayout) convertView
                    .getTag();
        }

        if (mDeviceThumbItemLayout != null && this.mDevicesList != null
                && position < this.mDevicesList.size() && position >= 0) {
            mDeviceThumbItemLayout.titleText.setText(this.mDevicesList.get(
                    position).getName());
            if (this.mDevicesList.get(position).isLocalDevice()) {
                mDeviceThumbItemLayout.thumbImage
                        .setImageResource(R.drawable.media_device_disk);
            } else {
                mDeviceThumbItemLayout.thumbImage
                        .setImageResource(R.drawable.media_device_dms);
            }
        }

        return convertView;
    }

    private class DeviceThumbItemLayout {
        ImageView thumbImage;
        TextView titleText;
    }

}
