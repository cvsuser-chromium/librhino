package com.hybroad.video;

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
import android.widget.ImageView.ScaleType;
import android.widget.TextView;

public class VideoThumbItemAdapter extends BaseAdapter {
  private Context mContext;
  private ArrayList<VideoFile> mFolderAndVideoList;
  private AsyncPosterLoader mAsyncPosterLoader;
  private GridView mGridView;

  public VideoThumbItemAdapter(Context context,
      ArrayList<VideoFile> folderAndVideoData, GridView gridView) {
    this.mContext = context;
    this.mFolderAndVideoList = folderAndVideoData;
    this.mGridView = gridView;
    this.mAsyncPosterLoader = new AsyncPosterLoader();
  }

  @Override
  public int getCount() {
    return (mFolderAndVideoList == null) ? 0 : mFolderAndVideoList.size();
  }

  @Override
  public Object getItem(int position) {
    return (mFolderAndVideoList == null) ? null : mFolderAndVideoList
        .get(position);
  }

  @Override
  public long getItemId(int position) {
    return position;
  }

  @Override
  public View getView(int position, View convertView, ViewGroup parent) {
    VideoThumbItemLayout mVideoThumbLayout = null;
    if (convertView == null) {
      mVideoThumbLayout = new VideoThumbItemLayout();
      convertView = LayoutInflater.from(mContext).inflate(
          R.layout.video_browser_thumbnail_item, null);
      mVideoThumbLayout.thumbImage = (ImageView) convertView
          .findViewById(R.id.iv_movie2nd_thumbnailitem);
      mVideoThumbLayout.titleText = (TextView) convertView
          .findViewById(R.id.tv_movie2nd_thumbnailitem);
      convertView.setTag(mVideoThumbLayout);
    } else {
      mVideoThumbLayout = (VideoThumbItemLayout) convertView.getTag();
    }

    if (mVideoThumbLayout != null && mFolderAndVideoList != null
        && position < mFolderAndVideoList.size() && position >= 0) {
      mVideoThumbLayout.titleText
          .setText(mFolderAndVideoList.get(position).title);
      if (mFolderAndVideoList.get(position).isFolder()) {
        mVideoThumbLayout.thumbImage.setImageResource(R.drawable.media_folder);
        mVideoThumbLayout.thumbImage.setScaleType(ScaleType.CENTER_INSIDE);
        // mVideoThumbLayout.thumbImage.setBackgroundColor(Color.TRANSPARENT);
      } else {
        mVideoThumbLayout.thumbImage.setScaleType(ScaleType.FIT_XY);
        String imageTag = mFolderAndVideoList.get(position).path;
        mVideoThumbLayout.thumbImage.setTag(imageTag);
        Drawable cachedImageDrawable = mAsyncPosterLoader
            .loadDrawableFromCache(imageTag);
        if (cachedImageDrawable == null) {
          mVideoThumbLayout.thumbImage
              .setImageResource(R.drawable.video_thumbnail_default);
        } else {
          mVideoThumbLayout.thumbImage.setImageDrawable(cachedImageDrawable);
        }
      }
    }

    return convertView;
  }

  class VideoThumbItemLayout {
    ImageView thumbImage;
    TextView titleText;
  }

  public Drawable loadCacheDrawable(String imageTag) {
    return mAsyncPosterLoader.loadDrawableFromCache(imageTag);
  }

  public synchronized void loadVideoDrawable(final int position,
      final String imageTag) {
    PosterCallback mAlbumImageCallback = new PosterCallback() {
      @Override
      public void imageLoaded(Drawable imageDrawable, String imageTag) {
        ImageView imageViewByTag = (ImageView) mGridView
            .findViewWithTag(imageTag);
        if (imageViewByTag != null) {
          imageViewByTag.setImageDrawable(imageDrawable);
        }
      }
    };
    mAsyncPosterLoader.loadDrawableFromVideo(position, imageTag,
        mAlbumImageCallback);
  }

}
