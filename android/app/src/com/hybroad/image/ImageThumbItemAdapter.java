package com.hybroad.image;


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

public class ImageThumbItemAdapter extends BaseAdapter {
    private Context mContext;
    private ArrayList<ImageFile> dirAndPictureList;
    private AsyncImageLoader asyncPhotoImageLoader;
    private GridView mGridView;
    
    public ImageThumbItemAdapter(Context context, ArrayList<ImageFile> directoryAndPictureData, GridView gridView) {
        this.mContext = context;
        this.dirAndPictureList = directoryAndPictureData;
        this.mGridView = gridView;
        this.asyncPhotoImageLoader = new AsyncImageLoader(context);
    }

    public int getCount() {
        return (dirAndPictureList == null) ? 0 : dirAndPictureList.size();
    }

    public Object getItem(int position) {
        return (dirAndPictureList == null) ? null : dirAndPictureList.get(position);
    }

    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        PictureThumbItemLayout mPictureThumbItemLayout = null;
        if (convertView == null) {
            mPictureThumbItemLayout = new PictureThumbItemLayout();
            convertView = LayoutInflater.from(mContext).inflate(R.layout.image_browser_thumbnail_item, null);
            mPictureThumbItemLayout.thumbImage = (ImageView)convertView.findViewById(R.id.picture_thumb_image);
            mPictureThumbItemLayout.titleText = (TextView)convertView.findViewById(R.id.picture_title);
            convertView.setTag(mPictureThumbItemLayout);
        } else {
            mPictureThumbItemLayout = (PictureThumbItemLayout)convertView.getTag();
        }

        if (mPictureThumbItemLayout != null && dirAndPictureList != null && position < dirAndPictureList.size() && position >= 0) {
        	mPictureThumbItemLayout.titleText.setText(dirAndPictureList.get(position).title);
        	if (dirAndPictureList.get(position).isFolder()) {
        		mPictureThumbItemLayout.thumbImage.setImageResource(R.drawable.media_folder);
        	} else {
        		String imageTag = dirAndPictureList.get(position).filePath;
        		mPictureThumbItemLayout.thumbImage.setTag(imageTag);
        		Drawable cachedImageDrawable = asyncPhotoImageLoader.loadDrawableFromCache(imageTag);
        		if (cachedImageDrawable == null) {
        			mPictureThumbItemLayout.thumbImage.setImageResource(R.drawable.image_thumbnail_default);
        		} else {
        			mPictureThumbItemLayout.thumbImage.setImageDrawable(cachedImageDrawable);
        		}
        	}
        }
        
        return convertView;
    }

    class PictureThumbItemLayout {
        ImageView thumbImage;
        TextView titleText;
    }

    public Drawable loadCacheDrawable(String imageTag) {
        return asyncPhotoImageLoader.loadDrawableFromCache(imageTag);
    }

    public synchronized void loadPhotoDrawable(final int position, final String imageTag) {
        ImageCallback mPhotoImageCallback = new ImageCallback() {
            @Override
            public void imageLoaded(Drawable imageDrawable, String imageTag) {
                ImageView imageViewByTag = (ImageView) mGridView.findViewWithTag(imageTag);
                if (imageViewByTag != null) {
                    imageViewByTag.setImageDrawable(imageDrawable);
                }
            }
        };
        asyncPhotoImageLoader.loadDrawableFromPhoto(position, imageTag, mPhotoImageCallback);
    }

}
