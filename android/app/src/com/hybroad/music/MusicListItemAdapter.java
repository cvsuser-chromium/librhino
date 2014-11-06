package com.hybroad.music;

import java.util.ArrayList;

import com.hybroad.media.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class MusicListItemAdapter extends BaseAdapter {
    private Context mContext;
    private ArrayList<MusicFile> audiosList;

    public MusicListItemAdapter(Context context, ArrayList<MusicFile> data) {
        mContext = context;
        audiosList = data;
    }

    @Override
    public int getCount() {
        return (audiosList == null) ? 0 : audiosList.size();
    }

    @Override
    public Object getItem(int position) {
        return (audiosList == null) ? null : audiosList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        AudioListItemLayout mAudioListItemLayout = null;
        if (convertView == null) {
            mAudioListItemLayout = new AudioListItemLayout();
            convertView = LayoutInflater.from(mContext).inflate(
                    R.layout.music_player_list_item, null);
            mAudioListItemLayout.selectImage = (ImageView) convertView
                    .findViewById(R.id.music_item_image);
            mAudioListItemLayout.titleText = (TextView) convertView
                    .findViewById(R.id.audio_list_title);
            convertView.setTag(mAudioListItemLayout);
        } else {
            mAudioListItemLayout = (AudioListItemLayout) convertView.getTag();
        }

        mAudioListItemLayout.titleText.setText((audiosList == null) ? null
                : audiosList.get(position).title);
        mAudioListItemLayout.selectImage
                .setBackgroundResource(R.drawable.music_play_item_focus_change);

        return convertView;
    }

    class AudioListItemLayout {
        ImageView selectImage;
        TextView titleText;
    }

}
