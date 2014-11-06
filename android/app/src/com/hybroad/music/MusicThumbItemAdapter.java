package com.hybroad.music;

import java.util.ArrayList;

import com.hybroad.media.R;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

public class MusicThumbItemAdapter extends BaseAdapter {
    private Context mContext;
    private ArrayList<MusicFile> dirAndAudioList;
    private AsyncAlbumLoader asyncAlbumImageLoader;
    private GridView mGridView;

    public MusicThumbItemAdapter(Context context,
            ArrayList<MusicFile> directoryAndAudioData, GridView gridView) {
        this.mContext = context;
        this.dirAndAudioList = directoryAndAudioData;
        this.mGridView = gridView;
        this.asyncAlbumImageLoader = new AsyncAlbumLoader();
    }

    @Override
    public int getCount() {
        return (dirAndAudioList == null) ? 0 : dirAndAudioList.size();
    }

    @Override
    public Object getItem(int position) {
        return (dirAndAudioList == null) ? null : dirAndAudioList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        AudioThumbItemLayout mAudioThumbLayout = null;
        if (convertView == null) {
            mAudioThumbLayout = new AudioThumbItemLayout();
            convertView = LayoutInflater.from(mContext).inflate(
                    R.layout.music_browser_thumbnail_item, null);
            mAudioThumbLayout.thumbImage = (ImageView) convertView
                    .findViewById(R.id.thumb_image);
            mAudioThumbLayout.titleText = (TextView) convertView
                    .findViewById(R.id.audio_title);
            convertView.setTag(mAudioThumbLayout);
        } else {
            mAudioThumbLayout = (AudioThumbItemLayout) convertView.getTag();
        }

        if (mAudioThumbLayout != null && dirAndAudioList != null
                && position < dirAndAudioList.size() && position >= 0) {
            mAudioThumbLayout.titleText
                    .setText(dirAndAudioList.get(position).title);
            if (dirAndAudioList.get(position).isFolder()) {
                mAudioThumbLayout.thumbImage
                        .setImageResource(R.drawable.media_folder);
            } else {
                String imageTag = dirAndAudioList.get(position).filePath;
                mAudioThumbLayout.thumbImage.setTag(imageTag);
                Drawable cachedImageDrawable = asyncAlbumImageLoader
                        .loadDrawableFromCache(imageTag);
                if (cachedImageDrawable == null) {
                    mAudioThumbLayout.thumbImage
                            .setImageResource(R.drawable.music_thumbnail_default);
                } else {
                    mAudioThumbLayout.thumbImage
                            .setImageDrawable(cachedImageDrawable);
                }
            }
        }

        return convertView;
    }

    class AudioThumbItemLayout {
        ImageView thumbImage;
        TextView titleText;
    }

    public Drawable loadCacheDrawable(String imageTag) {
        return asyncAlbumImageLoader.loadDrawableFromCache(imageTag);
    }

    public synchronized void loadSongDrawable(final int position,
            final String imageTag) {
        AlbumCallback mAlbumImageCallback = new AlbumCallback() {
            @Override
            public void imageLoaded(Drawable imageDrawable, String imageTag) {
                ImageView imageViewByTag = (ImageView) mGridView
                        .findViewWithTag(imageTag);
                if (imageViewByTag != null) {
                    imageViewByTag.setImageDrawable(imageDrawable);
                }
            }
        };
        asyncAlbumImageLoader.loadDrawableFromSong(position, imageTag,
                mAlbumImageCallback);
    }

}
