package com.hybroad.video;

import com.hybroad.media.R;

import android.app.Activity;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

public class MenuItemListAdapter extends BaseAdapter {
  private Activity mActivity;
  private String[] mMenuItems;

  public MenuItemListAdapter(Activity activity, String[] items) {
    this.mActivity = activity;
    this.mMenuItems = items;
  }

  @Override
  public int getCount() {
    return (null == mMenuItems) ? 0 : mMenuItems.length;
  }

  @Override
  public Object getItem(int position) {
    return (null == mMenuItems) ? null : mMenuItems[position];
  }

  @Override
  public long getItemId(int position) {
    return position;
  }

  @Override
  public View getView(int position, View convertView, ViewGroup parent) {
    LayoutInflater inflater = mActivity.getLayoutInflater();
    ViewHolder viewHolder = null;

    if (convertView == null) {
      convertView = inflater.inflate(R.layout.menu_list_item, null);
      viewHolder = new ViewHolder();
      viewHolder.textView = (TextView) convertView
          .findViewById(R.id.menu_list_item_1);
      convertView.setTag(viewHolder);
    } else {
      viewHolder = (ViewHolder) convertView.getTag();
    }

    viewHolder.textView.setTextColor(Color.WHITE);
    viewHolder.textView.setTextSize(20);
    viewHolder.textView.setText(mMenuItems[position]);

    return convertView;
  }

  class ViewHolder {
    TextView textView;
  }
}
